
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
#ifdef DEBUG
        std::cout <<  " BBS::init is_master=" << BBSImpl::is_master_ << " " << std::endl;
#endif
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
    execute_helper(); //builds and execute hoc statement
    //I need some way of Identifying what is said
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
#ifdef DEBUG
      std::cout << ParSpike::my_rank <<  ": in BBSImpl::worker() " << std::endl;
#endif
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
    terminate();
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
std::cout<< " BBSImpl::return_args( " <<  id<< " ):" << std::endl;
    i = upkint(); // userid
    int style = upkint();
std::cout<< " message userid= " <<  i<< "  style=" <<  style<< std::endl;
    switch (style) {
    case 0:
        s = upkstr(); // the statement
std::cout<< " statement | " <<  s<< " |" << std::endl;
        delete [] s;
        break;
    case 2: // obj first
        s = upkstr(); // template name
        i = upkint();   // instance index
std::cout<< " object  " <<  s<< " [" <<  i << "]"<< std::endl;
        delete [] s;
        //fall through
    case 1:
        s = upkstr(); //fname
        i = upkint(); // arg manifest
std::cout<<" fname="<< s <<" manifest= "<< i <<std::endl;
        delete [] s;
        break;
    }
    // now only args are left and ready to unpack.
}



/*! 
 * If the nid is equal to my_rank, push the gid onto the gid2in_ list 
 * 
 * @param gid neuron's id
 * @param nid rank of node
 */
void BBS::set_gid2node(int gid, int nid)
{
//    alloc_space();  commented out because gid2presyn tables are already defined

  if (nid == ParSpike::my_rank) {
#if DEBUG ==2
        std::cout<< " gid  " <<  gid<< "  defined on " <<  ParSpike::my_rank<< std::endl;
#endif
        //Clear GID in the incoming Gid2PreSyn table
        if (gid2in_->find(gid)->first)
            gid2in_->erase(gid);
	//Set NULL pointer GID in outgoing Gid2PreSyn table
     	//gid2out_->insert(std::pair<const int, PreSynPtr> (gid, nil));
	(*gid2out_)[gid] = nil;
    }
}



/*! 
 * Does the neuron, with \b gid, exist on this node
 * 
 * @param gid 
 * @return 
 */
int BBS::gid_exists(int gid)
{
    PreSynPtr ps;
    NetPar::alloc_space();
    if (ps = gid2out_->find(gid)->second) {
std::cout<<  my_rank <<" gid  " <<  gid<< "  exists"<< std::endl;
        if (ps) {
            return (ps->output_index_ >= 0 ? 3 : 2);
        } else {
            return 1;
        }
    }
    return 0;
}



/*! 
 * Set the threshold for spiking on neuron with gid
 * 
 * @param gid 
 * @param threshold 
 * @return the threshold of the neuron
 */
double BBS::threshold(int gid, double threshold)
{
    PreSynPtr ps = gid2out_->find(gid)->second;

    if (threshold == -1.0) {
        ps->threshold_ = threshold;
    }
    return ps->threshold_;

}




/*! 
 * Set the \b gid on the pre-synaptic pointer
 * 
 * @param gid 
 */
void BBS::cell(int gid) //, NetCon* nc) {
{
    PreSynPtr   ps = gid2out_->find(gid)->second;
    (*gid2out_)[gid] = ps;
    ps->gid_ = gid;
    ps->output_index_ = gid;
}


void BBS::outputcell(int gid)
{
    PreSynPtr ps = gid2out_->find(gid)->second;
//TODO  assert(ps = gid2out_->find(gid));
    assert(ps);
    ps->output_index_ = gid;
    ps->gid_ = gid;
}
/*
void BBS::spike_record(int gid, std::vector<double>* spikevec, std::vector<double>* gidvec) {
    PreSyn* ps;
    assert(gid2out_->find(gid, ps));
    assert(ps);
    ps->record(spikevec, gidvec, gid);
}
*/

/*! 
 * Get the reference pointer of Neuron with id \b gid 
 * 
 * @param gid 
 * @return ConfigBase pointer to Neuron
 */
ConfigBase* BBS::gid2obj(int gid)
{
    ConfigBase* cell = 0;
std::cout<<  my_rank <<" gid2obj gid= " <<  gid<< std::endl;
    PreSynPtr ps;
    assert(ps = gid2out_->find(gid)->second);
std::cout<< "  found " << std::endl;
//  assert(ps);
//TODO  cell = ps->ssrc_ ? ps->ssrc_->prop->dparam[6].obj : ps->osrc_;
std::cout<< "  return  " <<  (cell)<< std::endl;
    return cell;
}

ConfigBase* BBS::gid2cell(int gid)
{
    ConfigBase* cell = 0;
std::cout<<  my_rank <<" gid2obj gid= " <<  gid<< std::endl;
    PreSynPtr ps;
    assert(ps = gid2out_->find(gid)->second);
std::cout<< "  found " << std::endl;
    assert(ps);
    /*TODO  if (ps->ssrc_) {
            cell = ps->ssrc_->prop->dparam[6].obj;
        }else{
            cell = ps->osrc_;
            // but if it is a POINT_PROCESS in a section
            // that is inside an object ... (probably has a WATCH statement)
            Section* sec = ob2pntproc(cell)->sec;
            if (sec && sec->prop->dparam[6].obj) {
                cell = sec->prop->dparam[6].obj;
            }
        }
    */
//std::cout<< "  return  " <<  hoc_object_name(cell)<< std::endl;
    return cell;
}

ConfigBase* BBS::gid_connect(int gid, ConfigBase* target)
{
    /*  if (!is_point_process(target)) {
            hoc_execerror("arg 2 must be a point process", 0);
        }
    */
    NetPar::alloc_space();
    PreSynPtr ps;
    if (ps  = gid2out_->find(gid)->second) {
        // the gid is owned by this machine so connect directly
        assert(ps);
    } else if ((ps = gid2in_->find(gid)->second)) {
        // the gid stub already exists
std::cout<<  my_rank <<" connect  " <<  target->gid<< "  from already existing "<<  gid<< std::endl;
    } else {
std::cout<<  my_rank <<" connect  " <<  target->gid  << "  from new PreSyn for "<<  gid<< std::endl;
        PreSyn* ps_ = new PreSyn(nil, nil, nil);//,target);
        ps = PreSynPtr(ps_); //(nil, nil, nil);
//TODO?     net_cvode_instance->psl_append(ps);
        (*gid2in_)[gid] = ps;
        ps->gid_ = gid;
    }
    ConfigBase** po;
    /*TODO  NetCon* nc;

        if (ifarg(3)) {
            po = hoc_objgetarg(3);
            if (!*po || (*po)->ctemplate != netcon_sym_->u.ctemplate) {
                check_obj_type(*po, "NetCon");
            }
            nc = (NetCon*)((*po)->u.this_pointer);
            if (nc->target_ != ob2pntproc(target)) {
                std::cout <<"target is different from 3rd arg NetCon target" << std::endl;
            }
            nc->replace_src(ps);
        }else{
            nc = new NetCon(ps, target);
            po = hoc_temp_objvar(netcon_sym_, nc);
            nc->obj_ = *po;
        }
    */  return *po;

}

/*! Iterative step calling update on each neuron and plastic synapse
 * 
 * 
 * @param tstop termination time for simulation
 */
void BBS::netpar_solve(double tstop)
{

    double mt, md;
//  tstopunset;
    if (cvode_active_) {
        mt = 1e-9 ; md = NetPar::mindelay_;
    } else {
        mt = SimEnv::timestep() ; md = NetPar::mindelay_ - 1e-10;
    }
    if (md < mt) {
        if (my_rank == 0) {
            std::cout << "mindelay is 0 (or less than dt for fixed step method)" << std::endl;
        } else {
            return;
        }
    }
    double wt;

    NetPar::timeout(20);
    wt = wtime();
    if (cvode_active_) {
//TODO      ncs2nrn_integrate(tstop);
    } else {
//TODO      ncs2nrn_integrate(tstop+1e-11);
    }
    impl_->integ_time_ += wtime() - wt;
    impl_->integ_time_ -= (NetPar::npe_ ? (NetPar::npe_[0].wx_ + NetPar::npe_[0].ws_) : 0.);

    NetPar::spike_exchange();

    NetPar::timeout(0);
    impl_->wait_time_ += NetPar::wt_;
    impl_->send_time_ += NetPar::wt1_;
    if (NetPar::npe_) {
        impl_->wait_time_ += NetPar::npe_[0].wx_;
        impl_->send_time_ += NetPar::npe_[0].ws_;
        NetPar::npe_[0].wx_ = NetPar::npe_[0].ws_ = 0.;
    };
std::cout<< my_rank <<" netpar_solve exit t= " <<  SimEnv::sim_time()<< "  tstop=" << SimEnv::i_duration()*SimEnv::timestep() <<" mindelay_="<< NetPar::mindelay_ << std::endl;

//  tstopunset;
}


double BBS::netpar_mindelay(double maxdelay)
{
    return NetPar::set_mindelay(maxdelay);
}

void BBS::netpar_spanning_statistics(int* nsend, int* nsendmax, int* nrecv, int* nrecv_useful)
{
    *nsend = NetPar::nsend_;
    *nsendmax = NetPar::nsendmax_;
    *nrecv = NetPar::nrecv_;
    *nrecv_useful = NetPar::nrecv_useful_;
}

/*
std::vector<double> BBS::netpar_max_histogram(std::vector<double> mh)
{
  //TODO
        std::vector<double> h = NetPar::max_histogram_;
        if (NetPar::max_histogram_) {
            NetPar::max_histogram_ = nil;
        }
        if (mh) {
            NetPar::max_histogram_ = *mh;
        }
        return h;
  

}
*/
