// BBS.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BBS_H
#define BBS_H

#include <iostream>
#include <vector>

#ifdef CPPMPI
#include "ParSpike.2.h"
#include "BBS2MPI.2.h"
#else
#include "ParSpike.h"
#include "BBS2MPI.h"
#endif

#include "GlobalDefs.h"
#include "ConfigBase.h"
#include "InterfaceBase.h"

void bbs_done();

//! BBSImpl Bulletin Board Abstract class
/*! Primarily an abstract class that prepares server and client sides for BBS communication 
 */
class BBSImpl
{
public:
    BBSImpl();
    virtual ~BBSImpl();

    virtual bool look(const char*) = 0;

    virtual void take(const char*) = 0; /* blocks til something to take */
    virtual bool look_take(const char*) = 0; /* returns false if nothing to take */
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
    virtual bool working(int &id, double& x, int& userid);
    virtual void context();

    virtual void start();
    virtual void done();

    virtual void worker(); // forever execute
    virtual bool is_master();
    double time();

    virtual void perror(const char*);
public:
    double ac_;
    int working_id_, n_;
    double wait_time_;
    double integ_time_;
    double send_time_;
    static bool is_master_;
    static bool started_, done_;
    static int mytid_;
    static int debug_;
protected:
    void execute_helper(); // involves hoc specific details in ocbbs.c
};



//! BBS Base for all classes in Bulletin Board Server
/*! Takes care of the interface between ParSpike/MPI and ParNetwork/ParallelNetManager 
 */
class BBS : public ParSpike
{
public:
    BBS();
#ifdef CPPMPI
    BBS(int, int& argc, char**&argv);
#else
    BBS(int, int* argc, char***argv);
#endif
    virtual ~BBS();

    bool look(const char*);

    void take(const char*); /* blocks til something to take */
    bool look_take(const char*); /* returns false if nothing to take */
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
    bool working(int &id, double& x, int& userid);
    void context();

    bool is_master();
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
    double threshold(int, double);
    void cell();    void cell(int);
    void outputcell(int);
    void spike_record(int, std::vector<double>, std::vector<double>);
    void netpar_solve(double);
    ConfigBase* gid2obj(int);
    ConfigBase* gid2cell(int);
    ConfigBase* gid_connect(int, ConfigBase*);
    double netpar_mindelay(double maxdelay);
    void netpar_spanning_statistics(int*, int*, int*, int*);
    std::vector<double> netpar_max_histogram(std::vector<double>);
protected:
    void init(int);
protected:
    BBSImpl* impl_;
};


#endif
