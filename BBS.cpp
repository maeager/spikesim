
#include "BBS.h"
#include "BBServer.h"
#include <errno.h>

bool BBSImpl::is_master_ = false;
bool BBSImpl::started_ = false;
bool BBSImpl::done_ = false;

#undef DEBUG
#define DEBUG BBSImpl::debug_

int BBSImpl::debug_ = 0;
int BBSImpl::mytid_;

static int etaskcnt;
static double total_exec_time;
static double worker_take_time;

BBS::BBS()
{
    init(-1);
}

#ifdef CPPMPI
BBS::BBS(int n, int& pargc, char**&pargv)
{
    ParSpike::init(1, pargc, pargv);
    init(n);
}
#else
BBS::BBS(int n, int* pargc, char***pargv)
{
    ParSpike::init(1, pargc, pargv);
    init(n);
}
#endif




void BBS::init(int)
{
    if (mpi_use == 0) {
        BBSImpl::is_master_ = true;

        //impl_ = new BBSLocal();
        return;
    }
    if (!BBSImpl::started_) {

        BBSImpl::is_master_ = (my_rank == 0) ? true : false;
//std::cout <<  " BBS::init is_master=" <<  << " " <<  << std::endl;
    }
    // Just as with PVM which stored buffers on the bulletin board
    // so we have the following files to store MPI_PACKED buffers
    // on the bulletin board. It would be possible to factor out
    // the pvm stuff and we may do that later but for now we
    // start with copies of the four files that worked for PVM
    // and convert to the nrnmpi functions implemented in
    // ../nrnmpi
    // The four files are
    // bbsclimpi.cpp - mpi remote client BBSClient from bbsrcli.cpp
    // bbsdirectmpi.cpp - mpi master client BBSDirect from bbsdirect.cpp
    // bbssrvmpi.cpp - mpi server portion to remote client from master bbs
    //      BBSDirectServer derived from bbssrv.cpp
    // bbslsrvmpi.cpp - mpi master bbs portion of BBSDirectServer
    //      derived from bbslsrv2.cpp
    // We reuse the .h files of these.
    if (BBSImpl::is_master_) {
        impl_ = new BBSDirect();
    } else {
        impl_ = new BBSClient();
    }
}


BBSImpl::BBSImpl()
{
    wait_time_ = 0.;
    send_time_ = 0.;
    integ_time_ = 0.;
    working_id_ = 0;
    n_ = 0;
}

BBS::~BBS()
{
    delete impl_;
}

BBSImpl::~BBSImpl()
{
}

bool BBS::is_master()
{
    return BBSImpl::is_master_;
}

int BBS::nhost()
{
    return numprocs;
}

int BBS::myid()
{
    return my_rank;
}

bool BBSImpl::is_master()
{
    return is_master_;
}

double BBS::time()
{
    return impl_->time();
}

double BBSImpl::time()
{
    return ParSpike::wtime();

}

double BBS::wait_time()
{
    return impl_->wait_time_;
}
double BBS::integ_time()
{
    return impl_->integ_time_;
}
double BBS::send_time()
{
    return impl_->send_time_;
}
void BBS::add_wait_time(double st)
{
    impl_->wait_time_ += impl_->time() - st;
}

void BBS::perror(const char* s)
{
    impl_->perror(s);
}

void BBSImpl::perror(const char*)
{
}

int BBS::upkint()
{
    int i = impl_->upkint();
#ifdef DEBUG
    std::cout <<  " upkint " <<  i << std::endl;
#endif
    return i;
}

double BBS::upkdouble()
{
    double d = impl_->upkdouble();
#ifdef DEBUG
    std::cout <<  " upkdouble " <<  d << std::endl;
#endif
    return d;
}

void BBS::upkvec(int n, double* px)
{
    impl_->upkvec(n, px);
#ifdef DEBUG
    std::cout <<  " upkvec " <<  n << std::endl;
#endif
}

char* BBS::upkstr()
{
    char* s = impl_->upkstr();
#ifdef DEBUG
    std::cout <<  " upkstr " <<  s << std::endl;
#endif
    return s;
}

void BBS::pkbegin()
{
#ifdef DEBUG
    std::cout << "pkbegin\n";
#endif
    impl_->pkbegin();
}

void BBS::pkint(int i)
{
#ifdef DEBUG
    std::cout <<  " pkint " <<  i << std::endl;
#endif
    impl_->pkint(i);
}

void BBS::pkdouble(double x)
{
#ifdef DEBUG
    std::cout <<  " pkdouble " <<  x << std::endl;
#endif
    impl_->pkdouble(x);
}

void BBS::pkvec(int n, double* px)
{
#ifdef DEBUG
    std::cout <<  " pkdouble " <<  n << std::endl;
#endif
    impl_->pkvec(n, px);
}

void BBS::pkstr(const char* s)
{
#ifdef DEBUG
    std::cout <<  " pkstr " <<  s << std::endl;
#endif
    impl_->pkstr(s);
}

#if 0
// for now all todo messages are of the three item form
// tid
// gid
// id
// "stmt"
// the latter should set hoc_ac_ if the return value is important
// right now every arg must be literal, we'll handle variables later.
// eg the following should work.
// n = 1000  x = 0  for i=1,n { x += i }  hoc_ac_ = x
// although it may makes sense to send the result directly to the
// tid for now we just put it back onto the mailbox in the form
// message: "result tid gid" with two items, i.e. id, hoc_ac_
#endif
#if 0
// the todo management has been considerably modified to support
// a priority queue style for the order in which tasks are executed.
// Now the server manages the task id. Given a task id, it knows the
// parent task id.
// The working_id is always non-trivial on worker machines, and is
// only 0 on the master when dealing with a submission at the top level
// post_todo(working_id_) message is the statement, here the working_id is
//  the parent of the future stmt task.
// take_todo: message is the statement. return is the id of this task
// post_result(id) message is return value.
// the result will be retrieved relative to the submitting working_id_
// look_take_result(working_id_) message is the return value. the return
// value of this call is the id of the task that computed the return.
#endif

// BBSImpl::execute_helper() in ocbbs.c

void BBSImpl::execute(int id)   // assumes a "_todo" message in receive buffer
{
    ++etaskcnt;
    double st, et;
    int userid;
    char* s;
    int i;
    int save_id = working_id_;
    working_id_ = id;
    st = time();
    if (debug_) {
        std::cout <<  " execute begin " << st << " : working_id_= " <<  working_id_ <<  std::endl;
    }
    userid = upkint();
    ac_ = double(id);
    //  execute_helper(); //builds and execute hoc statement
    et = time() - st;
    total_exec_time += et;
#ifdef DEBUG
    std::cout <<  " execute end elapsed " << et << " : working_id_= " <<  working_id_ << "  ac_= " <<  ac_ <<  std::endl;
#endif
    pkbegin();
    pkint(userid);
    pkdouble(ac_);
    post_result(working_id_);
    working_id_ = save_id;
}

int BBS::submit(int userid)
{
    return impl_->submit(userid);
}

int BBSImpl::submit(int userid)
{
    // userid was the first item packed
    ++n_;
#ifdef DEBUG
    std::cout <<  " submit n_= " << n_ << "  for working_id= " <<  working_id_ << "  userid= " <<  userid <<  std::endl;
#endif
    if (userid < 0) {
        save_args(-userid);
    } else {
        post_todo(working_id_);
    }
    return userid;
}

void BBS::context()
{
    impl_->context();
}

void BBSImpl::context()
{
    std::cout << "can't execute BBS::context on a worker\n";
    exit(1);
}

bool BBS::working(int& id, double& x, int& userid)
{
    return impl_->working(id, x, userid);
}

bool BBSImpl::working(int& id, double& x, int& userid)
{
    int cnt = 0;
    double t;
    if (n_ <= 0) {
#ifdef DEBUG
        std::cout <<  " working n_= " <<  n_ << " : return false " << std::endl;
#endif
        return false;
    }
#ifdef DEBUG
    t = time();
#endif
    for (;;) {
        ++cnt;
        if ((id = look_take_result(working_id_)) != 0) {
            userid = upkint();
            x = upkdouble();
            --n_;
#ifdef DEBUG
            std::cout << "working n_=" << n_ << ": after " <<  cnt << " try elapsed " <<  time() - t << " sec got result for " << working_id_ << " id=" <<  id << " x=" << x << std::endl;
#endif
            if (userid < 0) {
                userid = -userid;
                return_args(userid);
            }
            return true;
        } else if ((id = look_take_todo()) != 0) {
#ifdef DEBUG
            std::cout <<  " working: no result for " <<  working_id_ << "  but did get _todo id=" << id  << " " << std::endl;
#endif
            execute(id);
        }
    }
};

void BBS::worker()
{
    impl_->worker();
}

void BBSImpl::worker()
{
    // forever request and execute commands
    double st, et;
    int id;
    if (!is_master()) {
        for (;;) {
            st = time();
            id = take_todo();
            et = time() - st;
            worker_take_time += et;
            execute(id);
        }
    }
}

void BBS::post(const char* key)
{
#ifdef DEBUG
    std::cout <<  " post: " <<  key << std::endl;
#endif
    impl_->post(key);
}

bool BBS::look_take(const char* key)
{
    bool b = impl_->look_take(key);
#ifdef DEBUG
    std::cout <<  " look_take |" << key << "| return " <<  b << std::endl;
#endif
    return b;
}

bool BBS::look(const char* key)
{
    bool b = impl_->look(key);
#ifdef DEBUG
    std::cout <<  " look |" << key << "| return " <<  b << std::endl;
#endif
    return b;
}

void BBS::take(const char* key)   // blocking
{
    double t;
#ifdef DEBUG
    t = time();
    std::cout <<  " begin take |" << key << "| at "
              <<  t << std::endl;
#endif
    impl_->take(key);
#ifdef DEBUG
    std::cout <<  " end take |" << key << "| elapsed "
              <<  time() - t << " from " << t << std::endl;
#endif
}

void BBS::done()
{
    impl_->done();
    //terminate();
}

void BBSImpl::done()
{
    if (done_) {
        return;
    }
    done_ = true;
#ifdef HAVE_TMS
    clock_t elapsed = times(&tmsbuf) - starttime;
    std::cout << "" << etaskcnt << " tasks in "
              << total_exec_time << " seconds. "
              <<  worker_take_time << " seconds waiting for tasks"
              << std::endl;

    std::cout << "user="
              << (double)(tmsbuf.tms_utime - tms_start_.tms_utime) / 100 << " sys="
              << (double)(tmsbuf.tms_stime - tms_start_.tms_stime) / 100 << " elapsed="
              << (double)(elapsed) / 100 << " "
              << 100.*(double)(tmsbuf.tms_utime - tms_start_.tms_utime) / (double)elapsed
              << std::endl;
    );
#endif
}

void BBSImpl::start()
{
    if (started_) {
        return;
    }
    started_ = 1;
#ifdef HAVE_TMS
    starttime = times(&tms_start_);
#endif
}


void BBSImpl::return_args(int id)
{
    // the message has been set up by the subclass
    // perhaps it would be better to do this directly
    // and avoid the meaningless create and delete.
    // but then they all would have to know this format
    int i;
    char* s;
//printf("BBSImpl::return_args(%d):\n", id);
    i = upkint(); // userid
    int style = upkint();
//printf("message userid=%d style=%d\n", i, style);
    switch (style) {
    case 0:
        s = upkstr(); // the statement
//printf("statement |%s|\n", s);
        delete [] s;
        break;
    case 2: // obj first
        s = upkstr(); // template name
        i = upkint();   // instance index
//printf("object %s[%d]\n", s, i);
        delete [] s;
        //fall through
    case 1:
        s = upkstr(); //fname
        i = upkint(); // arg manifest
//printf("fname=|%s| manifest=%o\n", s, i);
        delete [] s;
        break;
    }
    // now only args are left and ready to unpack.
}


