
#include <iostream>
#include <cstdio>
#include <map>
#include <set>
#include <utility>
#include <errno.h>

#include "BBS.h"
#include "BBServer.h"


#include <mpi.h>
#define POLLCNT 300

int bbs_poll_;
void bbs_handle();
//extern double hoc_cross_x_;
static int bbs_poll_cnt_;
static int bbs_msg_cnt_;

BBSDirectServer* BBSDirectServer::server_;


class WorkItem {
public:
	WorkItem(int id, bbsmpibuf* buf, int cid);
	virtual ~WorkItem();
	WorkItem* parent_;
	int id_;
	bbsmpibuf* buf_;
	int cid_; // mpi host id
	bool todo_less_than(const WorkItem*)const;
};

struct ltstr {
	bool operator() (const char* s1, const char* s2) const {
		return strcmp(s1, s2) < 0;
	}
};

struct ltint {
	bool operator() (int i, int j) const {
		return i < j;
	}
};

struct ltWorkItem {
	bool operator() (const WorkItem* w1, const WorkItem* w2) const {
		return w1->todo_less_than(w2);
	}
};

static char* newstr(const char* s) {
	char* s1 = new char[strlen(s) + 1];
	strcpy(s1, s);
	return s1;
}

WorkItem::WorkItem(int id, bbsmpibuf* buf, int cid) {
#if DEBUG == 2
printf("WorkItem %d\n", id);
#endif
	id_ = id;
	buf_ = buf;
	cid_ = cid;
	parent_ = nil;
}

WorkItem::~WorkItem() {
#if DEBUG
printf("~WorkItem %d\n", id_);
#endif
}

bool WorkItem::todo_less_than(const WorkItem* w) const {
	WorkItem* w1 = (WorkItem*)this;
	WorkItem* w2 = (WorkItem*)w;
	while (w1->parent_ != w2->parent_) {
		if (w1->id_ < w2->id_) {
			w2 = w2->parent_;
		}else{
			w1 = w1->parent_;
		}
	}
#if DEBUG
printf("todo_less_than %d < %d return %d\n", this->id_, w->id_, w1->id_ < w2->id_);
#endif
	return w1->id_ < w2->id_;
}

class KeepArgs : public std::map<int, bbsmpibuf*, ltint>{};


void bbs_handle() {
	if (BBSDirectServer::server_) {
		bbs_poll_ = POLLCNT;
	}else{
		bbs_poll_ = -1;
		return;
	}
	++bbs_poll_cnt_;
	BBSDirectServer::handle();
}




class MessageList : public std::multimap <const char*, bbsmpibuf*, ltstr>{};
class PendingList : public std::multimap <const char*, const int, ltstr>{};
class WorkList : public std::map <int, const WorkItem*, ltint>{};
class LookingToDoList : public std::set <int, ltint>{};
class ReadyList : public std::set<const WorkItem*, ltWorkItem>{};
class ResultList: public std::multimap<int, const WorkItem*, ltint>{};




BBSDirect::BBSDirect() {
	if (!BBSDirectServer::server_) {
		BBSDirectServer::server_ = new BBSDirectServer();
	}
	sendbuf_ = nil;
	recvbuf_ = nil;
	BBSDirect::start();

	keepargs_ = new KeepArgs();

}

BBSDirect::~BBSDirect() {
	BBS2MPI::unref(sendbuf_);
	BBS2MPI::unref(recvbuf_);

	delete keepargs_;

}

void BBSDirect::perror(const char* s) {
	printf("BBSDirect error %s\n", s);
}

void BBSDirect::context() {
	BBSDirectServer::handle();
	BBS2MPI::enddata(sendbuf_);
	BBSDirectServer::server_->context(sendbuf_);
	BBS2MPI::unref(sendbuf_);
	sendbuf_ = nil;
}

int BBSDirect::upkint() {
	int i;
	i = BBS2MPI::upkint(recvbuf_);
#if DEBUG
	printf("upkint returning %d\n", i);
#endif
	return i;
}

double BBSDirect::upkdouble() {
	double x;
	x = BBS2MPI::upkdouble(recvbuf_);
#if DEBUG
	printf("upkdouble returning %g\n", x);
#endif
	return x;
}

void BBSDirect::upkvec(int n, double* x) {
	BBS2MPI::upkvec(n, x, recvbuf_);
}

char* BBSDirect::upkstr() {
	char* s;
	s = BBS2MPI::upkstr(recvbuf_);
#if DEBUG
	printf("upkstr returning |%s|\n", s);
#endif
	return s;
}

void BBSDirect::pkbegin() {
#if DEBUG
//printf("%d BBSDirect::pkbegin\n", ParSpike::my_rank);
#endif
	BBS2MPI::unref(sendbuf_);
	sendbuf_ = BBS2MPI::newbuf(100);
	BBS2MPI::ref(sendbuf_);
	BBS2MPI::pkbegin(sendbuf_);
}

void BBSDirect::pkint(int i) {
#if DEBUG
//printf("%d BBSDirect::pkint %d\n", ParSpike::my_rank, i);
#endif
	BBS2MPI::pkint(i, sendbuf_);
}

void BBSDirect::pkdouble(double x) {
#if DEBUG
//printf("%d BBSDirect::pkdouble\n", ParSpike::my_rank, x);
#endif
	BBS2MPI::pkdouble(x, sendbuf_);
}

void BBSDirect::pkvec(int n, double* x) {
#if DEBUG
//printf("%d BBSDirect::pkvec n=%d\n", ParSpike::my_rank, n);
#endif
	BBS2MPI::pkvec(n, x, sendbuf_);
}

void BBSDirect::pkstr(const char* s) {
#if DEBUG
//printf("%d BBSDirect::pkstr %s\n", ParSpike::my_rank, s);
#endif
	BBS2MPI::pkstr(s, sendbuf_);
}

void BBSDirect::post(const char* key) {
#if DEBUG
//	printf("%d BBSDirect::post |%s|\n", ParSpike::my_rank, key);
#endif
	BBS2MPI::enddata(sendbuf_);
	BBS2MPI::pkstr(key, sendbuf_);
	BBSDirectServer::server_->post(key, sendbuf_);
	BBS2MPI::unref(sendbuf_);
	sendbuf_ = nil;
	BBSDirectServer::handle();
}

void BBSDirect::post_todo(int parentid) {
#if DEBUG
//	printf("%d BBSDirect::post_todo for %d\n", ParSpike::my_rank, parentid);
#endif
	BBS2MPI::enddata(sendbuf_);
	BBS2MPI::pkint(parentid, sendbuf_);
	BBSDirectServer::server_->post_todo(parentid, ParSpike::my_rank, sendbuf_);
	BBS2MPI::unref(sendbuf_);
	sendbuf_ = nil;
	BBSDirectServer::handle();
}

void BBSDirect::post_result(int id) {
#if DEBUG
//	printf("%d BBSDirect::post_result %d\n", ParSpike::my_rank, id);
#endif
	BBS2MPI::enddata(sendbuf_);
	BBS2MPI::pkint(id, sendbuf_);
	BBSDirectServer::server_->post_result(id, sendbuf_);
	BBS2MPI::unref(sendbuf_);
	sendbuf_ = nil;
	BBSDirectServer::handle();
}

int BBSDirect::look_take_todo() {
	BBSDirectServer::handle();
	int id = BBSDirectServer::server_->look_take_todo(&recvbuf_);
	if (id) {
		BBS2MPI::upkbegin(recvbuf_);
	
#if DEBUG
printf("%d look_take_todo getid=%d\n",ParSpike::my_rank, BBS2MPI::getid(recvbuf_));
#endif
	}
#if DEBUG
//printf("%d BBSDirect::look_take_todo id=%d\n", ParSpike::my_rank, id);
#endif
	return id;
}

int BBSDirect::take_todo() {
	int id = BBSDirectServer::server_->look_take_todo(&recvbuf_);
	if (id == 0) {
		printf("BBSDirect::take_todo blocking\n");
		assert(0);
	}
#if DEBUG
//	printf("%d BBSDirect::take_todo id=%d\n", ParSpike::my_rank, id);
#endif
	return id;
}

int BBSDirect::look_take_result(int pid) {
	BBSDirectServer::handle();
	int id = BBSDirectServer::server_->look_take_result(pid, &recvbuf_);
#if DEBUG
//	printf("%d BBSDirect::look_take_result id=%d pid=%d\n", ParSpike::my_rank, id, pid);
#endif
	if (id) {
		BBS2MPI::upkbegin(recvbuf_);
	}
#if DEBUG
//printf("%d look_take_result return id=%d\n", ParSpike::my_rank, id);
#endif
	return id;
}

void BBSDirect::save_args(int userid) {

	BBS2MPI::ref(sendbuf_);
	keepargs_->insert(
		std::pair<const int, bbsmpibuf*>(userid, sendbuf_)
	);
	

	post_todo(working_id_);
}

void BBSDirect::return_args(int userid) {

	KeepArgs::iterator i = keepargs_->find(userid);
	BBS2MPI::unref(recvbuf_);
	recvbuf_ = nil;
	if (i != keepargs_->end()) {
		recvbuf_ = (*i).second;
		keepargs_->erase(i);
		BBS2MPI::upkbegin(recvbuf_);
		BBSImpl::return_args(userid);
	}

}

bool BBSDirect::look_take(const char* key) {
	BBSDirectServer::handle();
	bool b = BBSDirectServer::server_->look_take(key, &recvbuf_);
	if (b) {
		BBS2MPI::upkbegin(recvbuf_);
	}
#if DEBUG
	if (b) {
		printf("look_take |%s| true\n", key);
	}else{
		printf("look_take |%s| false\n", key);
	}
#endif
	return b;
}

bool BBSDirect::look(const char* key) {
	BBSDirectServer::handle();
	bool b = BBSDirectServer::server_->look(key, &recvbuf_);
	if (b) {
		BBS2MPI::upkbegin(recvbuf_);
	}
#if DEBUG
	printf("look |%s| %d\n", key, b);
#endif
	return b;
}

void BBSDirect::take(const char* key) { // blocking
	int id;
	double st = time();
	for (;;) {
		if (look_take(key)) {
			wait_time_ += time() - st;
			return;
		} else if ((id = look_take_todo()) != 0) {
			wait_time_ += time() - st;
			execute(id);
			st = time();
		}else{
			// perhaps should do something meaningful here
			// like check whether to quit or not
		}
	}
}
	
void BBSDirect::done() {
	int i;
	if (done_) {
		return;
	}
	BBSImpl::done();
	done_ = true;
	BBS2MPI::unref(sendbuf_);
	sendbuf_ = BBS2MPI::newbuf(20);
#if DEBUG
printf("done: numprocs=%d\n", ParSpike::numprocs);
#endif
	for (i=1; i < ParSpike::numprocs; ++i) {
		BBS2MPI::bbssend(i, QUIT, sendbuf_);
//printf("kill %d\n", i);
	}
	BBSDirectServer::server_->done();
}

void BBSDirect::start() {
	if (started_) { return; }
	BBSImpl::start();
	is_master_ = true;
	BBSDirectServer::server_->start();
	bbs_handle();
}



void BBSDirectServer::start() {
	if (ParSpike::numprocs > 1) {
		bbs_poll_ = POLLCNT;
	}
}
void BBSDirectServer::done() {
#if DEBUG
	std::cout << "bbs_msg_cnt_="<< bbs_msg_cnt_<<" bbs_poll_cnt_=" << bbs_poll_cnt_ << " bbs_poll_=" <<  ((bbs_poll_ < 0) ? -bbs_poll_ : bbs_poll_) << std::endl;
#endif
	return;
}

void BBSDirectServer::handle() {
		int size;
		int tag;
		int source;
		if (BBS2MPI::iprobe(&size, &tag, &source) != 0) {
			do {
				handle1(size, tag, source);
			} while (BBS2MPI::iprobe(&size, &tag, &source) != 0);
		}
}

void BBSDirectServer::handle1(int size, int tag, int cid) {
	bbsmpibuf* recv;
	bbsmpibuf* send;
	char* key;
	int index;
	send = nil;
	recv = BBS2MPI::newbuf(size);
	BBS2MPI::ref(recv);
	BBS2MPI::bbsrecv(cid, recv);
	++bbs_msg_cnt_;
	if (size > 0) {
		BBS2MPI::upkbegin(recv);
	}
	switch (tag) {
	case POST_TODO:
		index = BBS2MPI::upkint(recv); // the parent index
#if DEBUG
std::cout << "handle POST_TODO from " << cid << std::endl;//<< " when cross=" << hoc_cross_x_ 
#endif
		BBSDirectServer::server_->post_todo(index, cid, recv);
		break;
	case POST_RESULT:
		index = BBS2MPI::getid(recv);
#if DEBUG
std::cout << "handle POST_RESULT " << index << " from " << cid << std::endl;//<< " when cross=" << hoc_cross_x_ 
#endif
		BBSDirectServer::server_->post_result(index, recv);
		break;
	case POST:
		key = BBS2MPI::getkey(recv);
#if DEBUG
std::cout << "handle POST " << key << " from " << cid << std::endl;//<< " when cross=" << hoc_cross_x_ 
#endif
		BBSDirectServer::server_->post(key, recv);
		break;
	case LOOK:
		key = BBS2MPI::getkey(recv);
#if DEBUG
std::cout << "handle LOOK " << key << " from " << cid  << std::endl;//<< " when cross=" << hoc_cross_x_
#endif
		if (BBSDirectServer::server_->look(key, &send)) {
			BBS2MPI::bbssend(cid, LOOK_YES, send);
			BBS2MPI::unref(send);
		}else{
			BBS2MPI::bbssend(cid, LOOK_NO, nil);
		}
		break;
	case LOOK_TAKE:
		key = BBS2MPI::getkey(recv);
#if DEBUG
		std::cout << "handle LOOK_TAKE " << key << " from " << cid  << std::endl;//<< " when cross=" << hoc_cross_x_
#endif
		if (BBSDirectServer::server_->look_take(key, &send)) {
#if DEBUG
std::cout << "handle sending back something" << std::endl;
#endif
			BBS2MPI::bbssend(cid, LOOK_TAKE_YES, send);
			BBS2MPI::unref(send);
		}else{
			BBS2MPI::bbssend(cid, LOOK_TAKE_NO, nil);
		}
		break;
	case TAKE:
		key = BBS2MPI::getkey(recv);
#if DEBUG
std::cout << "handle TAKE " << key << " from " << cid  << std::endl;//<< " when cross=" << hoc_cross_x_
#endif
		if (BBSDirectServer::server_->look_take(key, &send)) {
#if DEBUG
std::cout << "handle sending back something" << std::endl;
#endif
			BBS2MPI::bbssend(cid, TAKE, send);
			BBS2MPI::unref(send);
		}else{
#if DEBUG
std::cout << "handle put_pending " << key << " for " << cid << std::endl;
#endif
			BBSDirectServer::server_->put_pending(key, cid);
		}
		break;
	case LOOK_TAKE_TODO:
#if DEBUG
std::cout << "handle LOOK_TAKE_TODO for cid=" << cid << std::endl;
#endif
		index = BBSDirectServer::server_->look_take_todo(&send);
#if DEBUG
std::cout << "handle sending back id=" << index << std::endl;
#endif
		BBS2MPI::bbssend(cid, index+1, send);
		if (index) {
			BBS2MPI::unref(send);
		}			
		break;
	case LOOK_TAKE_RESULT:
		index = BBS2MPI::getid(recv);
#if DEBUG
std::cout << "handle LOOK_TAKE_RESULT for " << cid << " pid=" << index << std::endl;
#endif
		index = BBSDirectServer::server_->look_take_result(index, &send);
#if DEBUG
std::cout << "handle sending back id=" << index << std::endl;
#endif
		BBS2MPI::bbssend(cid, index+1, send);
		if (index) {
			BBS2MPI::unref(send);
		}
		break;
	case TAKE_TODO:
#if DEBUG
std::cout << "handle TAKE_TODO for " << cid << std::endl;
#endif
		if (server_->remaining_context_cnt_ > 0
		    && server_->send_context(cid)) {
#if DEBUG
std::cout << "handle sent back a context\n";
#endif
			break;
		}
		index = BBSDirectServer::server_->look_take_todo(&send);
		if (index) {
#if DEBUG
std::cout << "handle sending back id=" << index << std::endl;
#endif
			BBS2MPI::bbssend(cid, index+1, send);		
			BBS2MPI::unref(send);
		}else{
#if DEBUG
std::cout << "handle add_looking_todo" << std::endl;
#endif
			BBSDirectServer::server_->add_looking_todo(cid);
		}
		break;
	case HELLO:
#if DEBUG
std::cout << "handle HELLO from " << cid  << std::endl;//<< " when cross=" << hoc_cross_x_
#endif
		BBS2MPI::pkbegin(recv);
		BBS2MPI::enddata(recv);
		BBS2MPI::bbssend(cid, HELLO, recv);
		break;
	default:
std::cout << "unknown message" << std::endl;
		break;
	}
	BBS2MPI::unref(recv);
}








BBSDirectServer::BBSDirectServer(){

	messages_ = new MessageList();
	work_ = new WorkList();
	todo_ = new ReadyList();
	results_ = new ResultList();
	pending_ = new PendingList();
	looking_todo_ = new LookingToDoList();
	send_context_ = new LookingToDoList();
	next_id_ = FIRSTID;
	context_buf_ = nil;
	remaining_context_cnt_ = 0;


}

BBSDirectServer::~BBSDirectServer(){

	delete todo_;
	delete results_;
	delete looking_todo_;
std::cout << "~BBSLocalServer not deleting everything" << std::endl;
// need to unref MessageValue in messages_ and delete WorkItem in work_
	delete pending_;
	delete messages_;
	delete work_;
	delete send_context_;

}

bool BBSDirectServer::look_take(const char* key, bbsmpibuf** recv) {
#if DEBUG
	std::cout << "DirectServer::look_take |" << key << "|" << std::endl;
#endif
	bool b = false;

	BBS2MPI::unref(*recv);
	*recv = nil;
	MessageList::iterator m = messages_->find(key);
	if (m != messages_->end()) {
		b = true;
		*recv = (*m).second;
//std::cout << "free " <<  << "\n", buf);
		char* s = (char*)((*m).first);
		messages_->erase(m);
		delete [] s;
	}
#if DEBUG
		std::cout << "DirectServer::look_take |" << key << "| recv="<< (long)(*recv)<<" return " <<b  << std::endl;
#endif

	return b;
}

bool BBSDirectServer::look(const char* key, bbsmpibuf** recv) {
#if DEBUG
	std::cout << "DirectServer::look |" << key << "|" << std::endl;
#endif
	bool b = false;
	BBS2MPI::unref(*recv);
	*recv = nil;

	MessageList::iterator m = messages_->find(key);
	if (m != messages_->end()) {
		b = true;
		*recv  = (*m).second;
		if (*recv) {
			BBS2MPI::ref(*recv);
		}
	}
#if DEBUG
	std::cout << "DirectServer::look |" << key << "| recv="<< (long)(*recv)<<" return " <<  b << std::endl;
#endif

	return b;
}

void BBSDirectServer::put_pending(const char* key, int cid) {

#if DEBUG
std::cout << "put_pending |" <<key  << "| " << cid << std::endl;
#endif
	char* s = newstr(key);
	pending_->insert(std::pair<const char* const, const int>(s, cid));

}

bool BBSDirectServer::take_pending(const char* key, int* cid) {
	bool b = false;

	PendingList::iterator p = pending_->find(key);
	if (p != pending_->end()) {
		*cid = (*p).second;
#if DEBUG
std::cout << "take_pending |" << key << "| " << *cid<< std::endl;
#endif
		char* s =  (char*)((*p).first);
		pending_->erase(p);
		delete [] s;
		b = true;
	}

	return b;
}

void BBSDirectServer::post(const char* key, bbsmpibuf* send) {

	int cid;
#if DEBUG
	std::cout << "DirectServer::post |" << key << "| send=" << (long)send<< std::endl;
#endif
	if (take_pending(key, &cid)) {
		BBS2MPI::bbssend(cid, TAKE, send);
	}else{
		MessageList::iterator m = messages_->insert(
		  std::pair<const char* const, bbsmpibuf*>(newstr(key), send)
		);
		BBS2MPI::ref(send);
	}

}

void BBSDirectServer::add_looking_todo(int cid) {

	looking_todo_->insert(cid);

}

void BBSDirectServer::post_todo(int pid, int cid, bbsmpibuf* send){

#if DEBUG
std::cout << "BBSDirectServer::post_todo pid=" << pid << " cid=" << cid << " send=" <<  (long)send << std::endl;
#endif
	WorkItem* w = new WorkItem(next_id_++, send, cid);
	BBS2MPI::ref(send);
	WorkList::iterator p = work_->find(pid);
	if (p != work_->end()) {
		w->parent_ = (WorkItem*)((*p).second);
	}
	work_->insert(std::pair<const int, const WorkItem*>(w->id_, w));
#if DEBUG
std::cout << "work insert " <<w->id_  << std::endl;
#endif
	LookingToDoList::iterator i = looking_todo_->begin();
	if (i != looking_todo_->end()) {
		cid = (*i);
		looking_todo_->erase(i);
		// the send buffer is correct
		BBS2MPI::bbssend(cid, w->id_ + 1, send);
	}else{
#if DEBUG
std::cout << "todo insert\n";
#endif
		todo_->insert(w);
	}

}

void  BBSDirectServer::context(bbsmpibuf* send) {

	int cid, j;
#if DEBUG
std::cout << "numprocs=" << ParSpike::numprocs<<std::endl;
#endif
	if (remaining_context_cnt_ > 0) {
		std::cout << "some workers did not receive previous context" << std::endl;
		send_context_->erase(send_context_->begin(), send_context_->end());
		BBS2MPI::unref(context_buf_);
		context_buf_ = nil;
	}
	remaining_context_cnt_ = ParSpike::numprocs - 1;
	for (j = 1; j < ParSpike::numprocs; ++j) {
		send_context_->insert(j);
	}
	LookingToDoList::iterator i = looking_todo_->begin();
	while (i != looking_todo_->end()) {
		cid = (*i);
		looking_todo_->erase(i);
#if DEBUG
std::cout << "sending context to already waiting " <<cid  << std::endl;
#endif
		BBS2MPI::bbssend(cid, CONTEXT+1, send);
		i = send_context_->find(cid);
		send_context_->erase(i);
		--remaining_context_cnt_;
		i = looking_todo_->begin();
	}
	if (remaining_context_cnt_ > 0) {
		context_buf_ = send;
		BBS2MPI::ref(context_buf_);
		handle();
	}

}

void bbs_context_wait() {
	if (BBSImpl::is_master_) {
		BBSDirectServer::server_->context_wait();
	}
}

void BBSDirectServer::context_wait() {
//std::cout << "context_wait enter " << remaining_context_cnt_ << std::endl;
	while (remaining_context_cnt_) {
		handle();
	}
//std::cout << "context_wait exit " << remaining_context_cnt_ << std::endl;
}

bool  BBSDirectServer::send_context(int cid) {

	LookingToDoList::iterator i = send_context_->find(cid);
	if (i != send_context_->end()) {
		send_context_->erase(i);
#if DEBUG
std::cout << "sending context to " << cid << std::endl;
#endif
		BBS2MPI::bbssend(cid, CONTEXT+1, context_buf_);
		if (--remaining_context_cnt_ <= 0) {
			BBS2MPI::unref(context_buf_);
			context_buf_ = nil;
		}
		return true;
	}

	return false;
}

void BBSDirectServer::post_result(int id, bbsmpibuf* send){

#if DEBUG
std::cout << "DirectServer::post_result id=" << id << " send=" << (long)send << std::endl;
#endif
	WorkList::iterator i = work_->find(id);
	WorkItem* w = (WorkItem*)((*i).second);
	BBS2MPI::ref(send);
	BBS2MPI::unref(w->buf_);
	w->buf_ = send;
	results_->insert(std::pair<const int, const WorkItem*>(w->parent_ ? w->parent_->id_ : 0, w));

}

int BBSDirectServer::look_take_todo(bbsmpibuf** recv) {

#if DEBUG
std::cout << "DirectServer::look_take_todo" << std::endl;
#endif
	BBS2MPI::unref(*recv);
	*recv = nil;
	ReadyList::iterator i = todo_->begin();
	if (i != todo_->end()) {
		WorkItem* w = (WorkItem*)(*i);
		todo_->erase(i);
		*recv = w->buf_;
#if DEBUG
std::cout << "DirectServer::look_take_todo recv " << (long*)(*recv) << " with keypos=" << (*recv)->keypos << " return " << w->id_ << std::endl;
#endif
		w->buf_ = 0;
		return w->id_;
	}else{
		return 0;
	}
#
}

int BBSDirectServer::look_take_result(int pid, bbsmpibuf** recv) {
#if DEBUG
std::cout << "DirectServer::look_take_result pid=" << pid << std::endl;
#endif

	BBS2MPI::unref(*recv);
	*recv = nil;
	ResultList::iterator i = results_->find(pid);
	if (i != results_->end()) {
		WorkItem* w = (WorkItem*)((*i).second);
		results_->erase(i);
		*recv = w->buf_;
		int id = w->id_;
		WorkList::iterator j = work_->find(id);
		work_->erase(j);
		delete w;
#if DEBUG
std::cout << "DirectServer::look_take_result recv=" << (long)(*recv) << " return " << id << std::endl;
#endif
		return id;
	}else{
		return 0;
	}	

}

