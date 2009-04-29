#ifndef bbslsrv2_h
#define bbslsrv2_h

#include <nrnmpiuse.h>


class MpiMessageList;
class MpiPendingList;
class MpiWorkList;
class MpiReadyList;
class MpiLookingToDoList;
class MpiResultList;
struct bbsmpibuf;


	void bbs_handle();
	void bbs_done()

class BBSImpl {
public:
	BBSImpl();
	virtual ~BBSImpl();

	virtual boolean look(const char*) = 0;

	virtual void take(const char*) = 0; /* blocks til something to take */
	virtual boolean look_take(const char*) = 0; /* returns false if nothing to take */
	// after taking use these
	virtual int upkint() = 0;
	virtual double upkdouble() = 0;
	virtual void upkvec(int, double*) = 0;
	virtual char* upkstr() = 0; // delete [] char* when finished

	// before posting use these
	virtual void pkbegin() = 0;
	virtual void pkint(int) = 0;
	virtual void pkdouble(double) = 0;
	virtual void pkvec(int, double*) = 0;
	virtual void pkstr(const char*) = 0;
	virtual void post(const char*) = 0;

	virtual void post_todo(int parentid) = 0;
	virtual void post_result(int id) = 0;
	virtual int look_take_result(int pid) = 0; // returns id, or 0 if nothing
	virtual int look_take_todo() = 0; // returns id, or 0 if nothing
	virtual int take_todo() = 0; // returns id
	virtual void save_args(int userid) = 0;
	virtual void return_args(int userid);

	virtual void execute(int id); // assumes a "todo" message in receive buffer
	virtual int submit(int userid);
	virtual boolean working(int &id, double& x, int& userid);
	virtual void context();

	virtual void start();
	virtual void done();

	virtual void worker(); // forever execute
	virtual boolean is_master();
	virtual double time();
	
	virtual void perror(const char*);
public:
	int working_id_, n_;
	double wait_time_;
	double integ_time_;
	double send_time_;
	static boolean is_master_;
	static boolean started_, done_;
	static boolean use_pvm_;
	static int mytid_;
	static int debug_;
protected:
	void execute_helper(); // involves hoc specific details in ocbbs.c
};



class BBS {
public:
	BBS();
	BBS(int nhost);
	virtual ~BBS();

	boolean look(const char*);

	void take(const char*); /* blocks til something to take */
	boolean look_take(const char*); /* returns false if nothing to take */
	// after taking use these
	int upkint();
	double upkdouble();
	void upkvec(int n, double* px); // n input px space must exist
	char* upkstr(); // delete [] char* when finished

	// before posting use these
	void pkbegin();
	void pkint(int);
	void pkdouble(double);
	void pkvec(int n, double* px); // doesn't pack n
	void pkstr(const char*);
	void post(const char*);

	int submit(int userid);
	boolean working(int &id, double& x, int& userid);
	void context();

	boolean is_master();
	void worker(); // forever execute
	void done(); // prints timing

	void perror(const char*);
	double time();
	double wait_time();
	double integ_time();
	double send_time();
	void add_wait_time(double); // add interval since arg

	int nhost();
	int myid();

	// netpar interface
	void set_gid2node(int, int);
	int gid_exists(int);
	double threshold();
	void cell();
	void outputcell(int);
	void spike_record(int, IvocVect*, IvocVect*);
	void netpar_solve(double);
	Object** gid2obj(int);
	Object** gid2cell(int);
	Object** gid_connect(int);
	double netpar_mindelay(double maxdelay);
	void netpar_spanning_statistics(int*, int*, int*, int*);
	IvocVect* netpar_max_histogram(IvocVect*);
protected:
	void init(int);
protected:
	BBSImpl* impl_;
};


class BBS_Server {
public:
	BBS_Server();
	virtual ~BBS_Server();

	void post(const char* key, bbsmpibuf*);
	boolean look(const char* key, bbsmpibuf**);
	boolean look_take(const char* key, bbsmpibuf**);
	boolean take_pending(const char* key, int* cid);
	void put_pending(const char* key, int cid);
	static BBS_Server* server_;
	static void handle(); // all remote requests
	static void handle1(int size, int tag, int source);
	void start();
	void done();

	void post_todo(int parentid, int cid, bbsmpibuf*);
	void context(bbsmpibuf*);
	boolean send_context(int cid); // sends if not sent already
	void post_result(int id, bbsmpibuf*);
	int look_take_todo(bbsmpibuf**);
	int look_take_result(int parentid, bbsmpibuf**);
	void context_wait();
private:
	void add_looking_todo(int cid);
private:
	MpiMessageList* messages_;
	MpiPendingList* pending_;
	MpiWorkList* work_;
	MpiLookingToDoList* looking_todo_;
	MpiReadyList* todo_;
	MpiResultList* results_;
	MpiLookingToDoList* send_context_;
	int next_id_;
	bbsmpibuf* context_buf_;
	int remaining_context_cnt_;
};

#endif
