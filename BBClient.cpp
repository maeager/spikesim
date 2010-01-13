
#include <iostream>
#include <cstdio>
#include <map>
#include <set>
#include <utility>
#include <errno.h>


#include "BBS.h"
#include "BBServer.h"


struct ltint {
    bool operator()(int i, int j) const {
        return i < j;
    }
};

class KeepArgs : public std::map<int, bbsmpibuf*, ltint> {};

int BBSClient::sid_;

/*! Constructor for BBSClient
 * @name BBSClient - 
 * @return BBSClient
 */
BBSClient::BBSClient()
{
    sendbuf_ = nil;
    recvbuf_ = nil;
    request_ = BBS2MPI::newbuf(100);
    BBS2MPI::ref(request_);

    keepargs_ = new KeepArgs();

    BBSClient::start();
}
/**
 * @name BBSClient - 
 * @return void
 */
BBSClient::~BBSClient()
{
    BBS2MPI::unref(sendbuf_);
    BBS2MPI::unref(recvbuf_);
    BBS2MPI::unref(request_);

    delete keepargs_;

}

void BBSClient::perror(const char* s)
{
    printf("BBSClient error: %s\n", s);
}

void BBSClient::upkbegin()
{
    BBS2MPI::upkbegin(recvbuf_);
}

char* BBSClient::getkey()
{
    return BBS2MPI::getkey(recvbuf_);
}

int BBSClient::getid()
{
    return BBS2MPI::getid(recvbuf_);
}

int BBSClient::upkint()
{
    return BBS2MPI::upkint(recvbuf_);
}

double BBSClient::upkdouble()
{
    return BBS2MPI::upkdouble(recvbuf_);
}

void BBSClient::upkvec(int n, double* x)
{
    BBS2MPI::upkvec(n, x, recvbuf_);
}

char* BBSClient::upkstr()
{
    return BBS2MPI::upkstr(recvbuf_); // do not forget to free(string)
}

void BBSClient::pkbegin()
{
    if (!sendbuf_) {
        sendbuf_ = BBS2MPI::newbuf(100);
        BBS2MPI::ref(sendbuf_);
    }
    BBS2MPI::pkbegin(sendbuf_);
}

void BBSClient::pkint(int i)
{
    BBS2MPI::pkint(i, sendbuf_);
}

void BBSClient::pkdouble(double x)
{
    BBS2MPI::pkdouble(x, sendbuf_);
}

void BBSClient::pkvec(int n, double* x)
{
    BBS2MPI::pkvec(n, x, sendbuf_);
}

void BBSClient::pkstr(const char* s)
{
    BBS2MPI::pkstr(s, sendbuf_);
}

void BBSClient::post(const char* key)
{
#if DEBUG
    std::cout <<  ParSpike::my_rank << ": "
              <<  key << " BBSClient::post |"
              << key << "|" << std::endl;
#endif
    BBS2MPI::enddata(sendbuf_);
    BBS2MPI::pkstr(key, sendbuf_);
    BBS2MPI::bbssend(sid_, POST, sendbuf_);
    BBS2MPI::unref(sendbuf_);
    sendbuf_ = nil;
}

void BBSClient::post_todo(int parentid)
{
#if DEBUG
    std::cout <<  ParSpike::my_rank << ": " << parentid << " BBSClient::post_todo for %d " << std::endl;

#endif
    BBS2MPI::enddata(sendbuf_);
    BBS2MPI::pkint(parentid, sendbuf_);
    BBS2MPI::bbssend(sid_, POST_TODO, sendbuf_);
    BBS2MPI::unref(sendbuf_);
    sendbuf_ = nil;
}

void BBSClient::post_result(int id)
{
#if DEBUG
    std::cout <<  ParSpike::my_rank << ": " << id << " BBSClient::post_result %d " << std::endl;

#endif
    BBS2MPI::enddata(sendbuf_);
    BBS2MPI::pkint(id, sendbuf_);
    BBS2MPI::bbssend(sid_, POST_RESULT, sendbuf_);
    BBS2MPI::unref(sendbuf_);
    sendbuf_ = nil;
}

int BBSClient::get(const char* key, int type)
{
#if DEBUG
    std::cout <<  ParSpike::my_rank << ": BBSClient::get |" << key << " %s| type= " << type << std::endl;

#endif
    BBS2MPI::pkbegin(request_);
    BBS2MPI::enddata(request_);
    BBS2MPI::pkstr(key, request_);
    return get(type);
}

int BBSClient::get(int key, int type)
{
#if DEBUG
    std::cout <<  ParSpike::my_rank << ": BBSClient::get" << key << " type=%d " << type <<  std::endl;

#endif
    BBS2MPI::pkbegin(request_);
    BBS2MPI::enddata(request_);
    BBS2MPI::pkint(key, request_);
    return get(type) - 1; // sent id+1 so cannot be mistaken for QUIT
}

int BBSClient::get(int type)   // blocking
{

    fflush(stderr);
    double ts = time();
    BBS2MPI::unref(recvbuf_);
    recvbuf_ = BBS2MPI::newbuf(100);
    BBS2MPI::ref(recvbuf_);
    int msgtag = BBS2MPI::bbssendrecv(sid_, type, request_, recvbuf_);
    errno = 0;
    wait_time_ += time() - ts;
#if DEBUG
    std::cout <<  ParSpike::my_rank << ": " << msgtag << " BBSClient::get return msgtag=%d " << std::endl;

#endif
    if (msgtag == QUIT) {
        done();
    }
    return msgtag;
}

bool BBSClient::look_take(const char* key)
{
#if DEBUG
    std::cout <<  ParSpike::my_rank << ": " << key << " BBSClient::look_take %s " << std::endl;
#endif
    int type = get(key, LOOK_TAKE);
    bool b = (type == LOOK_TAKE_YES);
    if (b) {
        upkbegin();
    }
    return b;
}

bool BBSClient::look(const char* key)
{
#if DEBUG
    std::cout <<  ParSpike::my_rank << ": " << key << " BBSClient::look %s " << std::endl;
#endif
    int type = get(key, LOOK);
    bool b = (type == LOOK_YES);
    if (b) {
        upkbegin();
    }
    return b;
}

void BBSClient::take(const char* key)   // blocking
{
    int bufid;
    get(key, TAKE);
    upkbegin();
}

int BBSClient::look_take_todo()
{
    int type = get(0, LOOK_TAKE_TODO);
    if (type) {
        upkbegin();
    }
    return type;
}
/**
 * @name take_todo - Grabs todo buffers 
 * @return int
 */
int BBSClient::take_todo()
{
    int type;
    while ((type = get(0, TAKE_TODO)) == CONTEXT) {
        upkbegin();
        upkint(); // throw away userid
#if DEBUG
        std::cout <<  ParSpike::my_rank << " execute context " << std::endl;

#endif
        execute_helper();
    }
    upkbegin();
    return type;
}

int BBSClient::look_take_result(int pid)
{
    int type = get(pid, LOOK_TAKE_RESULT);
    if (type) {
        upkbegin();
    }
    return type;
}

void BBSClient::save_args(int userid)
{

    BBS2MPI::ref(sendbuf_);
    keepargs_->insert(
        std::pair<const int, bbsmpibuf* >(userid, sendbuf_)
    );


    post_todo(working_id_);
}

void BBSClient::return_args(int userid)
{

    KeepArgs::iterator i = keepargs_->find(userid);
    BBS2MPI::unref(recvbuf_);
    recvbuf_ = nil;
    if (i != keepargs_->end()) {
        recvbuf_ = (*i).second;
        BBS2MPI::ref(recvbuf_);
        keepargs_->erase(i);
        upkbegin();
        BBSImpl::return_args(userid);
    }

}
/**
 * @name done - Terminate the client
 * @return void
 */
void BBSClient::done()
{
#if DEBUG
    std::cout <<  ParSpike::my_rank << " BBSClient::done " << std::endl;
#endif
    BBSImpl::done();
    ParSpike::terminate();
    exit(0);
}
/**
 * @name start - Start the client
 * @return void
 */
void BBSClient::start()
{
    char* client = 0;
    int tid;
    int n;
    if (started_) {
        return;
    }
#if DEBUG
    std::cout <<  ParSpike::my_rank  << " BBSClient start " << std::endl;
#endif
    BBSImpl::start();
    sid_ = 0;
#if 0
    { // a worker
        is_master_ = false;
        BBS2MPI::pkbegin(request_);
        BBS2MPI::enddata(request_);
        assert(get(HELLO) == HELLO);
        return;
    }
#endif
}

