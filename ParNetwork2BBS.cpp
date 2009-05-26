
#include "ParSpike.h"
#include "BBS.h"


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


	static Object** gid2obj(int gid);
	static Object** gid2cell(int gid);
	static Object** gid_connect(int gid);



static void pack_help(int, OcBBS*);
static void unpack_help(int, OcBBS*);
static int submit_help(OcBBS*);
static char* key_help();


public:
	static bool posting_ = false;
	OcBBS* bbs;
}

ParNetwork2BBS::ParNetwork2BBS(){//Object*) {
	// not clear at moment what is best way to handle nested context
	int i = -1;
//	if (ifarg(1)) {
//		i = int(chkarg(1, 0, 10000));
//	}
	bbs = new OcBBS(1);
	bbs->ref();
}

ParNetwork2BBS::~ParNetwork2BBS(){
	bbs->unref();
}




static int ParNetwork2BBS::submit_help() {
	int id, i, firstarg, style;
	posting_ = true;
	bbs->pkbegin();
	i = 1;
	if (hoc_is_double_arg(i)) {
		bbs->pkint((id = (int)chkarg(i++, 0, 1e7)));
	}else{
		bbs->pkint((id = --bbs->next_local_));
	}
	if (ifarg(i+1)) {
#if 1
		int argtypes = 0;
		int ii = 1;
		if (hoc_is_str_arg(i)) {
			style = 1;
			bbs->pkint(style); // "fname", arg1, ... style
		}else{
			style = 2;
			bbs->pkint(style); // [object],"fname", arg1, ... style
			Object* ob = *hoc_objgetarg(i++);
			bbs->pkstr(ob->ctemplate->sym->name);
			bbs->pkint(ob->index);
//printf("ob=%s\n", hoc_object_name(ob));
		}
		bbs->pkstr(gargstr(i++));
		firstarg = i;
		for (; ifarg(i); ++i) { // first is least significant
			if (hoc_is_double_arg(i)) {
				argtypes += 1*ii;
			}else if (hoc_is_str_arg(i)) {
				argtypes += 2*ii;
			}else{ // must be a Vector
				argtypes += 3*ii;
			}
			ii *= 4;
		}
//printf("submit style %d %s argtypes=%o\n", style, gargstr(firstarg-1), argtypes);
		bbs->pkint(argtypes);
		pack_help(firstarg, bbs);
#endif
	}else{
		bbs->pkint(0); // hoc statement style
		bbs->pkstr(gargstr(i));
	}
	posting_ = false;
	return id;
}

static double submit() {
	int id;
	id = submit_help(bbs);
	bbs->submit(id);
	return double(id);
}
	
static double context() {
	submit_help(bbs);
//	printf("%d context %s %s\n", bbs->myid(), hoc_object_name(*hoc_objgetarg(1)), gargstr(2));
	bbs->context();
	return 1.;
}
	
static double working() {
	int id;
	bool b = bbs->working(id, bbs->retval_, bbs->userid_);
	if (b) {
		return double(id);
	}else{
		return 0.;
	}
}

static double retval() {
	return bbs->retval_;
}

static double userid() {
	return (double)bbs->userid_;
}

static double nhost() {
	return double(bbs->nhost());
}

static double worker() {
	bbs->worker();
	return 0.;
}

static double done() {
	bbs->done();
	return 0.;
}

static void pack_help(int i, OcBBS* bbs) {
	if (!posting_) {
		bbs->pkbegin();
		posting_ = true;
	}
	for (; ifarg(i); ++i) {
		if (hoc_is_double_arg(i)) {
			bbs->pkdouble(*getarg(i));
		}else if (hoc_is_str_arg(i)) {
			bbs->pkstr(gargstr(i));
		}else{
			int n; double* px;
			n = vector_arg_px(i, &px);
			bbs->pkint(n);
			bbs->pkvec(n, px);
		}
	}
}

static double pack() {
	pack_help(1, bbs);
	return 0.;
}

static double post(std::string key) {
	pack_help(2, bbs);
	posting_ = false;

	bbs->post(key);
	
	return 1.;
}

static void unpack_help(int i, OcBBS* bbs) {
	for (; ifarg(i); ++i) {
		if (hoc_is_pdouble_arg(i)) {
			*hoc_pgetarg(i) = bbs->upkdouble();
		}else if (hoc_is_str_arg(i)) {
			char* s = bbs->upkstr();
			char** ps = hoc_pgargstr(i);
			hoc_assign_str(ps, s);
			delete [] s;
		}else{
			Vect* vec = vector_arg(i);
			int n = bbs->upkint();
			vec->resize(n);
			bbs->upkvec(n, vec->vec());
		}
	}
}

static double unpack() {
	unpack_help(1, bbs);
	return 1.;
}

static double upkscalar() {
	return bbs->upkdouble();
}

static std::string upkstr() {
	char* s = bbs->upkstr();
	std::string ps = s;
	delete [] s;
	return ps;
}

static 	std::vector<double> upkvec(std::vector<double> vec=0) {

	int n = bbs->upkint();
	if (vec)) {
		vec.resize(n);
	}else{
		vec = new Vect(n);
	}
	bbs->upkvec(n, vec);
	return vec;
}

static char* key_help() {
	static char key[50];
	if (hoc_is_str_arg(1)) {
		return gargstr(1);
	}else{
		sprintf(key, "%g", *getarg(1));
		return key;
	}
}

static double take() {
	bbs->take(key_help());
	unpack_help(2, bbs);
	return 1.;
}

static double look() {
	if (bbs->look(key_help())) {
		unpack_help(2, bbs);
		return 1.;
	}
	return 0.;
}

static double look_take() {

	if (bbs->look_take(key_help())) {
		unpack_help(2, bbs);
		return 1.;
	}
	return 0.;
}

static double pctime() {
	return bbs->time();
}

static double vtransfer_time() {
/*	int mode = ifarg(1) ? int(chkarg(1, 0., 2.)) : 0;
	if (mode == 2) {
		return nrnmpi_rtcomp_time_;
#if PARANEURON
	}else if (mode == 1) {
		return nrnmpi_splitcell_wait_;
	}else{
		return nrnmpi_transfer_wait_;
	}
#else
	}
*/	
	return 0;
//#endif
	
}



static double wait_time() {
	return bbs->wait_time();
}

static double step_time() {
	double w =  bbs->integ_time();
#if PARANEURON
	w -= nrnmpi_transfer_wait_ + nrnmpi_splitcell_wait_;
#endif
	return w;
}

static double send_time() {
//	int arg = ifarg(1) ? int(chkarg(1, 0, 10)) : 0;
//	if (arg) {
//		return nrn_bgp_receive_time(arg);
//	}
	return bbs->send_time();
}

static double event_time() {
	return 0.;
}

static double integ_time() {
	return 0.;
}

static double set_gid2node() {
	bbs->set_gid2node(int(chkarg(1, 0, MD)), int(chkarg(2, 0, MD)));
	return 0.;
}

static double gid_exists(int gid) {
	return int(bbs->gid_exists(gid));
}

static double cell() {
	bbs->cell();
	return 0.;
}

static double threshold() {
	return bbs->threshold();
}

static double spcompress(int nspike=-1, int gid_compress=1,int xchng_meth = 0) {
	return (double)nrnmpi_spike_compress(nspike, gid_compress, xchng_meth);
}
/*
static double splitcell_connect(void* v) {
	int that_host = (int)chkarg(1, 0, ParSpike::numprocs-1);
	// also needs a currently accessed section that is the root of this_tree
	nrnmpi_splitcell_connect(that_host);
	return 0.;
}


static double multisplit(void* v) {
	double x = -1.;
	int sid = -1;
	int backbone_style = 2;
	int reducedtree_host = 0;
	if (ifarg(1)) {
		x = chkarg(1, 0, 1);
		sid = (int)chkarg(2, 0, (double)(0x7fffffff));
	}
	if (ifarg(3)) {
		backbone_style = (int)chkarg(3, 0, 2);
	}
	// also needs a currently accessed section
	nrnmpi_multisplit(x, sid, backbone_style);
	return 0.;
}
*/
static double gid_clear() {
	nrnmpi_gid_clear();
	return 0.;
}

static double outputcell(int gid) {
	bbs->outputcell(gid);
	return 0.;
}

static double spike_record(int gid, double* spikevec, double* gidvec) {
	bbs->spike_record(gid, spikevec, gidvec);
	return 0.;
}

static double psolve(double  step) {
	bbs->netpar_solve(step);
	return 0.;
}

static double set_maxstep(double maxstep) {
	return bbs->netpar_mindelay(maxstep);
}

static double spike_stat(int *nsend,int * nsendmax,int * nrecv, int *nrecv_useful ) {
	nsend = nsendmax = nrecv = nrecv_useful = 0;
	bbs->netpar_spanning_statistics(nsend, nsendmax, nrecv, nrecv_useful);
	return double(nsendmax);
}

static double maxhist(std::vector<double> vec) {
	bbs->netpar_max_histogram(vec);
	return 0.;
}

static double source_var(void*) { // &source_variable, source_global_index
	// At BEFORE BREAKPOINT, the value of variable is sent to the
	// target machine(s).  This can only be executed on the
	// source machine (where source_variable exists).
	nrnmpi_source_var();
	return 0.;
}

static double target_var(void*) { // &target_variable, source_global_index
	// At BEFORE BREAKPOINT, the value of the target_variable is set
	// to the value of the source variable associated
	// with the source_global_index.  This can only be executed on the
	// target machine (where target_variable exists).
	nrnmpi_target_var();
	return 0.;
}

static double setup_transfer(void*) { // after all source/target and before init and run
	nrnmpi_setup_transfer();
	return 0.;
}

static double barrier(void*) {
	// return wait time
	double t = 0.;

	if (ParSpike::numprocs > 1) {
		t = nrnmpi_wtime();
		nrnmpi_barrier();
		t = nrnmpi_wtime() - t;
	}
	errno = 0;
	return t;
}

static double allreduce(double val , int type) {
	// type 1,2,3 sum, max, min
	double val = *getarg(1);

	if (ParSpike::numprocs > 1) {
		int type = (int)chkarg(2, 1, 3);
		val = nrnmpi_dbl_allreduce(val, type);
	}
	errno = 0;
	return val;
}

static double allgather(double val, std::vector<double> vec) {
	
	vec.resize(ParSpike::numprocs);

	if (ParSpike::numprocs > 1) {
		nrnmpi_dbl_allgather(&val, vec, 1);
		errno = 0;
	}else{
		vec[0] = val;
	}

	return 0.;
}

static double alltoall( std::vector<double> vsrc, std::vector<double> vscnt, std::vector<double> vdest) {
	int i, ns, np = ParSpike::numprocs;
	ns = vsrc.capacity();
	
	if (vscnt.capacity() != np) {
		throw ConfigError("alltoall(): size of source counts vector is not nhost");
	}
	
	int* scnt = new int[np];
	int* sdispl = new int[np+1];
	sdispl[0] = 0;
	for (i=0; i < np; ++i) {
		scnt[i] = int(vscnt[i]);
		sdispl[i+1] = sdispl[i] + scnt[i];
	}
	if (ns != sdispl[np]) {
		throw ConfigError("alltoall(): sum of source counts is not the size of the src vector");
	}


	int* rcnt = new int[np];
	int* rdispl = new int[np + 1];
	int* c = new int[np];
	rdispl[0] = 0;
	for (i=0; i < np; ++i) {
		c[i] = 1;
		rdispl[i+1] = i+1;
	}
	ParSpike::int_alltoallv(scnt, c, rdispl, rcnt, c, rdispl);
	delete [] c;
	for (i=0; i < np; ++i) {
		rdispl[i+1] = rdispl[i] + rcnt[i];
	}
	vdest.resize(rdispl[np]);
	double* r = vector_vec(vdest);
	ParSpike::dbl_alltoallv(vsrc, scnt, sdispl, r, rcnt, rdispl);
	delete [] rcnt;
	delete [] rdispl;
	delete [] scnt;
	delete [] sdispl;
	return 0.;
}

static double broadcast(std::string &s, int srcid) {
	if (srcid >=  ParSpike::numprocs || srcid < 0) {
		throw ConfigError("broadcast(): srcid is not in numprocs range");
	}
	int cnt = 0;

    if (ParSpike::numprocs > 1) {
		if (srcid == ParSpike::my_rank) {
			cnt = s.size();
		}
		ParSpike::int_broadcast(&cnt, 1, srcid);
		if (srcid != ParSpike::my_rank) {
			s.resize(cnt);
		}
		ParSpike::char_broadcast(s, cnt, srcid);
		if (srcid != ParSpike::my_rank) {
		//	hoc_assign_str(hoc_pgargstr(1), s);
			s.resize(0);
		}
    return double(s.size());
}

static double broadcast(std::vector<double> &vec, int srcid) {
	if (srcid >=  ParSpike::numprocs || srcid < 0) {
		throw ConfigError("broadcast(): srcid is not in numprocs range");
	}
	int cnt = 0;

    if (ParSpike::numprocs > 1) {

		if (srcid == ParSpike::my_rank) {
			cnt = vec.capacity();
		}
		ParSpike::int_broadcast(&cnt, 1, srcid);
		if (srcid != ParSpike::my_rank) {
			vec.resize(cnt);
		}
		ParSpike::dbl_broadcast(vec, cnt, srcid);
	}
	return double(cnt);
}

/*
static double checkpoint(void*) {
#if BLUEGENE_CHECKPOINT
	int i = BGLCheckpoint();
	return double(i);
#else
	return 0.;
#endif
}


static double nthrd(void*) {
	int ip = 1;
	if (ifarg(1)) {
		if (ifarg(2)) { ip = int(chkarg(2, 0, 1)); }
		nrn_threads_create(int(chkarg(1, 1, 1e5)), ip);
	}
	return double(nrn_nthread);
}
*/

/*
static double partition(void*) {
	Object* ob = 0;
	int it;
	if (ifarg(2)) {
		ob = *hoc_objgetarg(2);
		if (ob) {
			check_obj_type(ob, "SectionList");
		}
	}
	if (ifarg(1)) {
		it = (int)chkarg(1, 0, nrn_nthread - 1);
		nrn_thread_partition(it, ob);
	}else{
		for (it = 0; it < nrn_nthread; ++it) {
			nrn_thread_partition(it, ob);
		}
	}
	return 0.0;
}

static double thread_stat(void*) {
	nrn_thread_stat();
	return 0.0;
}

static double thread_busywait(void*) {
	int old = nrn_allow_busywait(int(chkarg(1,0,1)));
	return double(old);
}

static double thread_how_many_proc(void*) {
	int i = nrn_how_many_processors();
	return double(i);
}

static double sec_in_thread(void*) {
	Section* sec = chk_access();
	return double(sec->pnode[0]->_nt->id);
}

static double thread_ctime(void*) {
	int i;
#if 1
	if (ifarg(1)) {
		i = int(chkarg(1, 0, nrn_nthread));
		return nrn_threads[i]._ctime;
	}else{
		for (i=0; i < nrn_nthread; ++i) {
			nrn_threads[i]._ctime = 0.0;
		}
	}
#endif
	return 0.0;
}
*/

/* Must convert these to Synaptic interface */
static Object** gid2obj(int gid) {
	return bbs->gid2obj(gid);
}

static Object** gid2cell(int gid) {
	return bbs->gid2cell(gid);
}

static Object** gid_connect(int gid) {
	return bbs->gid_connect(gid);
}


/*
void BBSImpl::execute_helper() {
	char* s;
	int style = upkint();
	switch (style) {
	case 0:
		s = upkstr();
		//hoc_obj_run(s, nil);
//!!run command
		delete [] s;
		break;
	case 1:
	case 2: {
		int i, j;
		Symbol* fname;
		Object* ob = nil;
		char* sarg[20]; // upto 20 argument may be strings
		int ns = 0; // number of args that are strings
		int narg = 0; // total number of args
		if (style == 2) { // object first
			s = upkstr(); // template name
			i = upkint(); // object index
//printf("template |%s| index=%d\n", s, i);
			Symbol* sym = hoc_lookup(s);
			if (sym) {
				sym = hoc_which_template(sym);
			}
			if (!sym) {
				hoc_execerror(s, "is not a template");
			}
			hoc_Item* q, *ql;
			ql = sym->u.ctemplate->olist;
			ITERATE(q, ql) {
				ob = OBJ(q);
				if (ob->index == i) {
					break;
				}
				ob = nil;
			}
			if (!ob) {
fprintf(stderr, "%s[%d] is not an Object in this process\n", s, i);
hoc_execerror("ParallelContext execution error", 0);
			}
			delete [] s;
			s = upkstr();
			fname = hoc_table_lookup(s, sym->u.ctemplate->symtable);
		}else{
			s = upkstr();
			fname = hoc_lookup(s);
		}
//printf("execute helper style %d fname=%s obj=%s\n", style, fname->name, hoc_object_name(ob));
		if (!fname) {
fprintf(stderr, "%s not a function in %s\n", s, hoc_object_name(ob));
hoc_execerror("ParallelContext execution error", 0);
		}
		int argtypes = upkint(); // first is least signif
		for (j = argtypes; (i = j%4) != 0; j /= 4) {
			++narg;
			if (i == 1) {
				double x = upkdouble();
//printf("arg %d scalar %g\n", narg, x);
				hoc_pushx(x);
			}else if (i == 2) {
				sarg[ns] = upkstr();
//printf("arg %d string |%s|\n", narg, sarg[ns]);
				hoc_pushstr(sarg+ns);
				ns++;
			}else{
				int n;
				n = upkint();
				std::vector<double> vec(n);
//printf("arg %d vector size=%d\n", narg, n);
				upkvec(n, vec);
				hoc_pushobj(vec->temp_objvar());
			}
		}
		hoc_ac_ = hoc_call_objfunc(fname, narg, ob);
		delete [] s;
		for (i=0; i < ns; ++i) {
			delete [] sarg[i];
		}

	    }
		break;
	}
}
*/

void BBSImpl::return_args(int id) {
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
		i = upkint();	// instance index
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

