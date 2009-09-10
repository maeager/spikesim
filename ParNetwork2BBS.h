#ifndef PARNETWORK2BBS_H
#define PARNETWORK2BBS_H



#ifdef CPPMPI
#include "ParSpike.2.h"
#else
#include "ParSpike.h"
#endif
#include "ParNetwork.h"
#include "BBS.h"
#include "NetPar.h"

#undef MD
#define MD 2147483648.





/*extern "C" {

//  Symbol* hoc_which_template(Symbol*);

//  extern double t;

}
*/


class OcBBS : public BBS  //, public Resource {
{
public:
#ifdef CPPMPI
    OcBBS(int n, int& pargc, char**&pargv): BBS(n, pargc, pargv) {
        next_local_ = 0;
    }
#else
    OcBBS(int n, int* pargc, char***pargv): BBS(n, pargc, pargv) {
        next_local_ = 0;
    }
#endif

    virtual ~OcBBS() {}

    double retval_;
    int userid_;
    int next_local_;
};



class ParNetwork2BBS
{
public:
#ifdef CPPMPI
    ParNetwork2BBS(int& argc, char**&argv);
#else
    ParNetwork2BBS(int* argc, char***argv);
#endif
    ~ParNetwork2BBS();

    int submit_help();
    double submit();
    double working();
    double retval();
    double userid();
    double pack();
    double post(std::string);
    double unpack();
    double upkscalar();
    double take();
    double look();
    double look_take();
    double worker();
    double done();
    int id();
    int nhost();
    double context();

    double pctime();
    double wait_time();
    double step_time();
    double send_time();
    double event_time();  //empty
    double integ_time(); //empty
    double vtransfer_time(); //empty
//no     mech_time

    double set_gid2node(int gid, int nid);
    double gid_exists(int gid);
    double outputcell(int gid) ;
    double cell();
    double threshold(int, double);
//   double spike_record(int gid, double* spikevec, double* gidvec) ;
    double psolve(double  step);
    double set_maxstep(double maxstep);
    double spike_stat(int *nsend, int * nsendmax, int * nrecv, int *nrecv_useful);
    double maxhist(std::vector<double> vec);
    double checkpoint(void*);
    double spcompress(int , int , int);
    double gid_clear();

    double source_var(void*);  // &source_variable, source_global_index
    double target_var(void*) ; // &target_variable, source_global_index
    double setup_transfer(void*); // after all source/target and before init and run
//  "splitcell_connect", splitcell_connect,
//  "multisplit", multisplit,

    double barrier();
    double allreduce(double val , int type) ;
    double allgather(double val, std::vector<double> *vec) ;
    double alltoall(std::vector<double> *vsrc, std::vector<double> *vscnt, std::vector<double> *vdest);
    double broadcast(std::string s, int srcid) ;
    double broadcast(std::vector<double> vec, int srcid) ;
//  "nthread", nthrd,
//  "partition", partition,
//  "thread_stat", thread_stat,
//  "thread_busywait", thread_busywait,
//  "thread_how_many_proc", thread_how_many_proc,
//  "sec_in_thread", sec_in_thread,
//  "thread_ctime", thread_ctime,


    std::string upkstr();
    double upkvec(std::vector<double>*);


    ConfigBase* gid2obj(int gid);
    NeuronInterface* gid2cell(int gid);
    ConfigBase* gid_connect(int gid, ConfigBase*);



    void pack_help(int);
    void unpack_help(int);
    char* key_help();



public:
    static bool posting_;
    OcBBS* bbs;
};

#endif
