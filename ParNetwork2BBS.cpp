/*!
  This file is based directly on NEURON's "src/parallel/ocbbs.cpp"
  */


#ifdef CPPMPI
#include "ParSpike.2.h"
#else
#include "ParSpike.h"
#endif
#include "BBS.h"
#include "ParNetwork2BBS.h"
#include "NetPar.h"
#include "AnyBuf.h"

#include <string>
#include <vector>
#include <deque>
#include <boost/any.hpp>
#include <errno.h>

extern int errno;


bool ParNetwork2BBS::posting_ = false;
#ifdef CPPMPI
ParNetwork2BBS::ParNetwork2BBS(int& argc, char**&argv)
{
#else
ParNetwork2BBS::ParNetwork2BBS(int* argc, char***argv)
{
#endif
    // not clear at moment what is best way to handle nested context
    int i = -1;
//  if (ifarg(1)) {
//      i = int(chkarg(1, 0, 10000));
//  }
    bbs = new OcBBS(1, argc, argv);

    //bbs->ref();
}

ParNetwork2BBS::~ParNetwork2BBS()
{
    //bbs->unref();
}

int ParNetwork2BBS::id()
{
    return int(bbs->myid());
}

int ParNetwork2BBS::submit_help()
{
    int id, i, firstarg, style;
    posting_ = true;
    bbs->pkbegin();
    i = 1;
    if (double *d = getval(i)) {
        bbs->pkint((id = (int)(*d))); i++;
    } else {
        bbs->pkint((id = --bbs->next_local_));
    }
    if (ifarg(i+1)) {
	int argtypes = 0;
	int ii = 1;
	if (char **c = boost::any_cast<char*> (&bbsbuf[i])){
	     style = 1;
	     bbs->pkint(style); // "fname", arg1, ... style
        }
/* TODO not sure what to do here just yet -- Assume arg is stringx
       else if(ConfigBase *b = boost::any_cast<ConfigBase> (&bbsbuf[i])) {
	     style = 2;
	     bbs->pkint(style); // [object],"fname", arg1, ... style
	    //Object* ob = *hoc_objgetarg(i++);
	     i++;
	    bbs->pkstr(b->name.c_str());
	    bbs->pkint(b->index);
    //std::cout<< " ob= " <<  hoc_object_name(ob)<< std::endl;
	}
*/	std::string *s = boost::any_cast<std::string> (&bbsbuf[i++]);
            bbs->pkstr(s->c_str());
            firstarg = i;
            for (; ifarg(i); ++i) { // first is least significant
		if (double *d = boost::any_cast<double> (&bbsbuf[i])) {
                    argtypes += 1*ii;
		}else if (std::string *s = boost::any_cast<std::string> (&bbsbuf[i++])){
                    argtypes += 2*ii;
                }else{ // must be a Vector
                    argtypes += 3*ii;
                }
                ii *= 4;
            }
    //printf("submit style %d %s argtypes=%o\n", style, gargstr(firstarg-1), argtypes);
    //        bbs->pkint(argtypes);
    //        pack_help(firstarg, bbs);

	    bbs->pkint(argtypes);
	    pack_help(firstarg);
        }else{
            bbs->pkint(0); // hoc statement style
	    std::string* s = boost::any_cast<std::string> (&bbsbuf[i++]);
	    bbs->pkstr(s->c_str());//gargstr(i));
        }
    posting_ = false;
    return id;
}

double ParNetwork2BBS::submit()
{
    int id;
    id = submit_help();
    bbs->submit(id);
    return double(id);
}

double ParNetwork2BBS::context()
{
    submit_help();
#ifdef DEBUG
std::cout << bbs->myid() <<" ParNetwork2BBS::context " <<std::endl; //<<gargstr(2)
#endif
bbs->context();
    return 1.;
}

double ParNetwork2BBS::working()
{
    int id;
    bool b = bbs->working(id, bbs->retval_, bbs->userid_);
    if (b) {
        return double(id);
    } else {
        return 0.;
    }
}

double ParNetwork2BBS::retval()
{
    return bbs->retval_;
}

double ParNetwork2BBS::userid()
{
    return (double)bbs->userid_;
}

int ParNetwork2BBS::nhost()
{
    return int(bbs->nhost());
}

double ParNetwork2BBS::worker()
{
    bbs->worker();
    return 0.;
}

double ParNetwork2BBS::done()
{
    bbs->done();
    return 0.;
}

void ParNetwork2BBS::pack_help(int i)
{
    if (!posting_) {
        bbs->pkbegin();
        posting_ = true;
    }
    for (; ifarg(i); ++i) {
        if (int *a = getint(i)) {
            bbs->pkint(*a);
        } else if (double *d = getval(i)) {
            bbs->pkdouble(*d);
	} else if (std::string *s = getstr(i)) {
            bbs->pkstr(s->c_str());
        } else {
            if (std::vector<double> *vec = getvec(i)) {
                int n = vec->size();
                bbs->pkint(n);
                bbs->pkvec(n, &(*vec)[0]);
            }
        }
    }
}

double ParNetwork2BBS::pack()
{
    pack_help(1);
    return 0.;
}

double ParNetwork2BBS::post(std::string key)
{
    pack_help(2);
    posting_ = false;
    bbs->post(key.c_str());

    return 1.;
}

void ParNetwork2BBS::unpack_help(int i)
{
    std::vector<double> *vec;
    for (; ifarg(i); ++i) {
        if (double *pdouble = getval(i)) {
            *pdouble = bbs->upkdouble();
        } else if (std::string *ps = getstr(i)) {
            char* s = bbs->upkstr();
            *ps = s;
            delete [] s;
        } else {
            if ((vec = getvec(i))) {
                int n = bbs->upkint();
                vec->resize(n);
                bbs->upkvec(n, &(*vec)[0]);
            }
        }
    }

}

double ParNetwork2BBS::unpack()
{
    unpack_help(1);
    return 1.;
}

double ParNetwork2BBS::upkscalar()
{
    return bbs->upkdouble();
}

std::string ParNetwork2BBS::upkstr()
{
    char* s = bbs->upkstr();
    std::string ps = s;
    delete [] s;
    return ps;
}

double ParNetwork2BBS::upkvec(std::vector<double>* vec)
{

    int n = bbs->upkint();
    vec->resize(n);
    bbs->upkvec(n, &(*vec)[0]);
    return 1.;
}

char* ParNetwork2BBS::key_help()
{

    if (std::string *s = getstr(1)) {
        return &(*s)[0]; //sgargstr(1);
    } else {
        if (char **c = boost::any_cast<char*>(&bbsbuf[1])) return * c;
    }
    std::cout << id() << " key_help Error" << std::endl;
    return 0;
}

double ParNetwork2BBS::take()
{
    bbs->take(key_help());
    unpack_help(2);
    return 1.;
}

double ParNetwork2BBS::look()
{
    if (bbs->look(key_help())) {
        unpack_help(2);
        return 1.;
    }
    return 0.;
}

double ParNetwork2BBS::look_take()
{

    if (bbs->look_take(key_help())) {
        unpack_help(2);
        return 1.;
    }
    return 0.;
}

double ParNetwork2BBS::pctime()
{
    return bbs->time();
}

double ParNetwork2BBS::vtransfer_time()
{
    /*  int mode = ifarg(1) ? int(chkarg(1, 0., 2.)) : 0;
        if (mode == 2) {
            return bbs->rtcomp_time_;
    #if PARANEURON
        }else if (mode == 1) {
            return bbs->splitcell_wait_;
        }else{
            return bbs->transfer_wait_;
        }
    #else
        }
    */
    return 0;
//#endif

}

double ParNetwork2BBS::wait_time()
{
    return bbs->wait_time();
}

double ParNetwork2BBS::step_time()
{
    double w =  bbs->integ_time();
#if PARANEURON
    w -= bbs->transfer_wait_ + bbs->splitcell_wait_;
#endif
    return w;
}

double ParNetwork2BBS::send_time()
{
//  int arg = ifarg(1) ? int(chkarg(1, 0, 10)) : 0;
//  if (arg) {
//      return nrn_bgp_receive_time(arg);
//  }
    return bbs->send_time();
}

double ParNetwork2BBS::event_time()
{
    return 0.;
}

double ParNetwork2BBS::integ_time()
{
    return 0.;
}

double ParNetwork2BBS::set_gid2node(int gid, int nid)
{
    bbs->set_gid2node(gid, nid);//int(chkarg(1, 0, MD)), int(chkarg(2, 0, MD)));
    return 0.;
}

double ParNetwork2BBS::gid_exists(int gid)
{
    return int(bbs->gid_exists(gid));
}

double ParNetwork2BBS::cell()
{
    bbs->cell();
    return 0.;
}

double ParNetwork2BBS::threshold(int id, double thd = -1.0)
{
    return bbs->threshold(id, thd);
}

double ParNetwork2BBS::spcompress(int nspike = -1, int gid_compress = 1, int xchng_meth = 0)
{
    return (double)NetPar::spike_compress(nspike, gid_compress, xchng_meth);
}
/*
 double splitcell_connect(void* v) {
    int that_host = (int)chkarg(1, 0, bbs->numprocs-1);
    // also needs a currently accessed section that is the root of this_tree
    bbs->splitcell_connect(that_host);
    return 0.;
}


 double multisplit(void* v) {
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
    bbs->multisplit(x, sid, backbone_style);
    return 0.;
}
*/
double ParNetwork2BBS::gid_clear()
{
    NetPar::gid_clear();
    return 0.;
}

double ParNetwork2BBS::outputcell(int gid)
{
    bbs->outputcell(gid);
    return 0.;
}
/*
 double ParNetwork2BBS::spike_record(int gid, std::vector<double>* spikevec, std::vector<double>* gidvec) {
    //bbs->spike_record(gid, spikevec, gidvec);
    return 0.;
}
*/
double ParNetwork2BBS::psolve(double  stop)
{
    bbs->netpar_solve(stop);
    return 0.;
}

double ParNetwork2BBS::set_maxstep(double maxstep)
{
    return bbs->netpar_mindelay(maxstep);
}

double ParNetwork2BBS::spike_stat(int *nsend, int * nsendmax, int * nrecv, int *nrecv_useful)
{
    *nsend = *nsendmax = *nrecv = *nrecv_useful = 0;
    bbs->netpar_spanning_statistics(nsend, nsendmax, nrecv, nrecv_useful);
    return double(*nsendmax);
}

double ParNetwork2BBS::maxhist(std::vector<double> vec)
{
    bbs->netpar_max_histogram(vec);
    return 0.;
}

double ParNetwork2BBS::source_var(void*)   // &source_variable, source_global_index
{
    // At BEFORE BREAKPOINT, the value of variable is sent to the
    // target machine(s).  This can only be executed on the
    // source machine (where source_variable exists).
//  bbs->source_var();
    return 0.;
}

double ParNetwork2BBS::target_var(void*)   // &target_variable, source_global_index
{
    // At BEFORE BREAKPOINT, the value of the target_variable is set
    // to the value of the source variable associated
    // with the source_global_index.  This can only be executed on the
    // target machine (where target_variable exists).
//  bbs->target_var();
    return 0.;
}

double ParNetwork2BBS::setup_transfer(void*)   // after all source/target and before init and run
{
//    bbs->setup_transfer();
    return 0.;
}

double ParNetwork2BBS::barrier()
{
    // return wait time
    double t = 0.;
    if (bbs->numprocs > 1) {
        t = bbs->wtime();
        bbs->barrier();
        t = bbs->wtime() - t;
    }
    errno = 0;
    return t;
}

double ParNetwork2BBS::allreduce(double val , int type)
{
    // type 1,2,3 sum, max, min

    if (bbs->numprocs > 1) {
        val = bbs->dbl_allreduce(val, type);
    }
    errno = 0;
    return val;
}

double ParNetwork2BBS::allgather(double val, std::vector<double> *vec)
{

    vec->resize(bbs->numprocs);
    if (bbs->numprocs > 1) {
        bbs->dbl_allgather(&val, &(*vec)[0], 1);
        errno = 0;
    } else {
        (*vec)[0] = val;
    }
    return 0.;
}

double ParNetwork2BBS::alltoall(std::vector<double> *vsrc, std::vector<double> *vscnt, std::vector<double> *vdest)
{
    int i, ns, np = bbs->numprocs;
    ns = vsrc->capacity();

    if (vscnt->capacity() != np) {
	throw ConfigError("ParNetwork2BBS::alltoall(): size of source counts vector is not nhost");
    }

    int* scnt = new int[np];
    int* sdispl = new int[np+1];
    sdispl[0] = 0;
    for (i = 0; i < np; ++i) {
        scnt[i] = int((*vscnt)[i]);
        sdispl[i+1] = sdispl[i] + scnt[i];
    }
    if (ns != sdispl[np]) {
	throw ConfigError("ParNetwork2BBS::alltoall(): sum of source counts is not the size of the src vector");
    }


    int* rcnt = new int[np];
    int* rdispl = new int[np + 1];
    int* c = new int[np];
    rdispl[0] = 0;
    for (i = 0; i < np; ++i) {
        c[i] = 1;
        rdispl[i+1] = i + 1;
    }
    bbs->int_alltoallv(scnt, c, rdispl, rcnt, c, rdispl);
    delete [] c;
    for (i = 0; i < np; ++i) {
        rdispl[i+1] = rdispl[i] + rcnt[i];
    }
    vdest->resize(rdispl[np]);
    bbs->dbl_alltoallv(&(*vsrc)[0], scnt, sdispl, &(*vdest)[0], rcnt, rdispl);
    delete [] rcnt;
    delete [] rdispl;
    delete [] scnt;
    delete [] sdispl;
    return 0.;
}

double ParNetwork2BBS::broadcast(std::string s, int srcid)
{
    if (srcid >=  bbs->numprocs || srcid < 0) {
	throw ConfigError("ParNetwork2BBS::broadcast(): srcid is not in numprocs range");
    }
    int cnt = 0;
    std::string *ss;
    if (bbs->numprocs > 1) {
        if (srcid == bbs->my_rank) {
            cnt = s.size();
        }
        bbs->int_broadcast(&cnt, 1, srcid);
        if (srcid != bbs->my_rank) {
            s.resize(cnt);
        }
        bbs->char_broadcast(&s[0], cnt, srcid);
        if (srcid != bbs->my_rank  && (ss = boost::any_cast<std::string>(&bbsbuf[1]))) {
            *ss = s; //copy data across
            //  hoc_assign_str(hoc_pgargstr(1), s);
            s.resize(0);
        }
    }
    return double(s.size());
}

double ParNetwork2BBS::broadcast(std::vector<double> vec, int srcid)
{
    if (srcid >=  bbs->numprocs || srcid < 0) {
        throw ConfigError("broadcast(): srcid is not in numprocs range");
    }
    int cnt = 0;

    if (bbs->numprocs > 1) {

        if (srcid == bbs->my_rank) {
            cnt = vec.capacity();
        }
        bbs->int_broadcast(&cnt, 1, srcid);
        if (srcid != bbs->my_rank) {
            vec.resize(cnt);
        }
        bbs->dbl_broadcast(&vec[0], cnt, srcid);
    }
    return double(cnt);
}

/*
 double checkpoint(void*) {
#if BLUEGENE_CHECKPOINT
    int i = BGLCheckpoint();
    return double(i);
#else
    return 0.;
#endif
}


 double nthrd(void*) {
    int ip = 1;
    if (ifarg(1)) {
        if (ifarg(2)) { ip = *getint(2)); }
        nrn_threads_create(*getint(1), ip);
    }
    return double(nrn_nthread);
}
*/

/*
 double partition(void*) {
    Object* ob = 0;
    int it;
    if (ifarg(2)) {
        ob = *hoc_objgetarg(2);
        if (ob) {
            check_obj_type(ob, "SectionList");
        }
    }
    if (ifarg(1)) {
        it = (int)(*getint(1));
        nrn_thread_partition(it, ob);
    }else{
        for (it = 0; it < nrn_nthread; ++it) {
            nrn_thread_partition(it, ob);
        }
    }
    return 0.0;
}

 double thread_stat(void*) {
    nrn_thread_stat();
    return 0.0;
}

 double thread_busywait(void*) {
    int old = nrn_allow_busywait(*getint(1));
    return double(old);
}

 double thread_how_many_proc(void*) {
    int i = nrn_how_many_processors();
    return double(i);
}

 double sec_in_thread(void*) {
    Section* sec = chk_access();
    return double(sec->pnode[0]->_nt->id);
}

 double thread_ctime(void*) {
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
ConfigBase* ParNetwork2BBS::gid2obj(int gid)
{
    return (ConfigBase*)bbs->gid2obj(gid);
}

NeuronInterface* ParNetwork2BBS::gid2cell(int gid)
{
    return (NeuronInterface*)bbs->gid2cell(gid);
}

ConfigBase* ParNetwork2BBS::gid_connect(int gid, ConfigBase* syn)
{
    return (ConfigBase*)bbs->gid_connect(gid, syn);
}


//! Execute command 
/** Need to fit this command to ParSpikeSim or bypass it through direct commands
 */
void BBSImpl::execute_helper()
{
    char* s;
    int style = upkint();
    switch (style) {
    case 0:
        s = upkstr();
        hoc_obj_run(s, nil);
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
        std::cout<< " template | " <<  s<< " | index=" <<  i<< std::endl;
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
        std::cout<< " execute helper style  " <<  style<< "  fname=" <<  fname->name << " obj=" <<  hoc_object_name(ob)<< std::endl;
                if (!fname) {
        fprintf(stderr, "%s not a function in %s\n", s, hoc_object_name(ob));
        hoc_execerror("ParallelContext execution error", 0);
                }
                int argtypes = upkint(); // first is least signif
                for (j = argtypes; (i = j%4) != 0; j /= 4) {
                    ++narg;
                    if (i == 1) {
                        double x = upkdouble();
        std::cout<< " arg  " <<  narg<< "  scalar " <<  x<< std::endl;
                        hoc_pushx(x);
                    }else if (i == 2) {
                        sarg[ns] = upkstr();
        std::cout<< " arg  " <<  narg<< "  string |" <<  sarg[ns] << "|"<< std::endl;
                        hoc_pushstr(sarg+ns);
                        ns++;
                    }else{
                        int n;
                        n = upkint();
                        std::vector<double> vec(n);
        std::cout<< " arg  " <<  narg<< "  vector size=" <<  n<< std::endl;
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

