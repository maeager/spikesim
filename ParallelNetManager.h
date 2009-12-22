#ifndef PARALLELNETMANAGER_H
#define PARALLELNETMANAGER_H

#ifdef CPPMPI
#include "ParSpike.2.h"
#else
#include "ParSpike.h"
#endif
#include "ParNetwork.h"
//#include "NetPar.h"
#include "ParNetwork2BBS.h" 


#include <list>
#include <fstream>
#include <string>
#include <boost/shared_ptr.hpp>

#include "IdCounter.h"
#include "Error.h"
#include "DataCommonNeuron.h"
#include "DataRecordNeuron.h"
#include "TypeDefs.h"
#include "GlobalDefs.h"

class Group;

class  ParallelNetManager
{
public:
#ifdef CPPMPI
    ParallelNetManager(int& argc, char**&argv);
#else
    ParallelNetManager(int* argc, char***argv);
#endif
    ~ParallelNetManager();
    void init(int, int);
    void create_network(ParNetwork&);
    void connect_network(ParNetwork&net, bool);
    void register_cell(int, Group*);
    void synmech_append(int, int);
    void set_gid2node(int, int);
    bool gid_exists(int);
    void create_cell(int, Group*);

    ParNetwork2BBS* pc;

    void gatherspikes();
    void want_all_spikes();
    void spike_record(int);
    void doinit();
    void prun(), pcontinue(double), pinit(), psolve(double);
    void postwait(int x);
    void load_balance_round_robin(); //simplistic partitioning
    void load_balance_roulette();
    void load_balance_by_group();
    void done();

    ConfigBase* cm2t(int precell_id, ConfigBase* postcell_syn, double weight, double delay);
// mostly for debugging
    std::vector<double> spikevec;
    std::vector<double> idvec;
    std::map<int, boost::shared_ptr<NeuronInterface> >  cells;
    std::list<boost::shared_ptr<ConfigBase*>  >  synlist;
    void maxstepsize(), set_maxstep();
    int myid, ncell, nwork, nhost, ngroup, ncellgrp;
    int prstat, maxstepsize_called_, want_graph_, edgecount_;
    double localmaxstep_;
    double tstop;
    ConfigBase * nc;  //temp pointer in several member functions
    static int cell_cnt;
//external stdinit, continuerun, cvode, tstop, hoc_sf_
//external cvode_active, cvode_local
};

#endif
