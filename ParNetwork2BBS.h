
#include "ParSpike.h"
#include "ParNetwork.h"
#include "BBS.h"
#include "NetPar.h"

#undef MD
#define MD 2147483648.

extern "C" {

//	Symbol* hoc_which_template(Symbol*);

	extern double t;

}

class OcBBS : public BBS , public Resource {
public:
	OcBBS(int nhost_request);
	virtual ~OcBBS();
public:
	double retval_;
	int userid_;
	int next_local_;
};

OcBBS::OcBBS(int n) : BBS(n) {
	next_local_ = 0;
}

OcBBS::~OcBBS() {
}

class ParNetwork2BBS {
public:
	ParNetwork2BBS();
	~ParNetwork2BBS();

static int submit_help(OcBBS*);
	static double submit(); 
	static double working();
	static double retval();
	static double userid();
	static double pack();
	static double post();
	static double unpack();
	static double upkscalar();
	static double take();
	static double look();
	static double look_take();
	static double worker();
	static double done();

	static double nhost();
	static double context();

	static double pctime();
	static double wait_time();
	static double step_time(); 
	static double send_time();
	static double event_time();  //empty
	static double integ_time(); //empty
	static double vtransfer_time(); //empty
//no	 mech_time

	static double set_gid2node(int gid, int nid);
	double gid_exists(int gid);
	static double outputcell(int gid) ;
	static double cell();
	static double threshold();
	static double spike_record(int gid, double* spikevec, double* gidvec) ;
	static double psolve(double  step);
	static double set_maxstep(double maxstep);
	static double spike_stat(int *nsend,int * nsendmax,int * nrecv, int *nrecv_useful );
	static double maxhist(std::vector<double> vec);
	static double checkpoint(void*);
	static double spcompress(int nspike=-1, int gid_compress=1,int xchng_meth = 0);
	static double gid_clear() ;

	static double source_var(void*);  // &source_variable, source_global_index
	static double target_var(void*) ; // &target_variable, source_global_index
	static double setup_transfer(void*); // after all source/target and before init and run
//	"splitcell_connect", splitcell_connect,
//	"multisplit", multisplit,

	static double barrier(void*);
	static double allreduce(double val , int type) ;
	static double allgather(double val, std::vector<double> vec) ;
	static double alltoall( std::vector<double> vsrc, std::vector<double> vscnt, std::vector<double> vdest); 
	static double broadcast(std::string &s, int srcid) ;
static double broadcast(std::vector<double> &vec, int srcid) ;
//	"nthread", nthrd,
//	"partition", partition,
//	"thread_stat", thread_stat,
//	"thread_busywait", thread_busywait,
//	"thread_how_many_proc", thread_how_many_proc,
//	"sec_in_thread", sec_in_thread,
//	"thread_ctime", thread_ctime,


	std::string upkstr() ;
	std::vector<double> upkvec(std::vector<double>);


	static SynapseInterface* gid2obj(int gid);
	static NeuronInterface* gid2cell(int gid);
	static SynapseInterface* gid_connect(int gid);



	static void pack_help(int, OcBBS*);
	static void unpack_help(int, OcBBS*);
	static char* key_help();

public:
	static bool posting_ = false;
	OcBBS* bbs;
};
