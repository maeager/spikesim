


#include <errno.h>
#include "Group.h"
#include "GlobalDefs.h"
#include "AnyBuf.h"
#include "NeuronFactory.h"
#include "ParallelNetManager.h"
#include "ParNetwork2BBS.h"
//#include "NetPar.h"
#define nil 0

int ParallelNetManager::cell_cnt = 0;

/*! ParallelNetManager Constructor
 * Pass niput args to ParNetwork2BBS
 * @param argc pass input arguments to MPI_Init 
 * @param argv pass input arguments to MPI_Init 
 * 
 * @return 
 */
#ifdef CPPMPI
ParallelNetManager::ParallelNetManager(int& argc, char**&argv)
#else
ParallelNetManager::ParallelNetManager(int* argc, char***argv)
#endif
{
    pc = new ParNetwork2BBS(argc, argv);
    init(1, 1);

}

//!Destructor
ParallelNetManager::~ParallelNetManager()
{
}

 
/*! 
 * sets the default parameters for PNM
 * 
 * @param ncells 
 * @param ngroups 
 */void ParallelNetManager::init(int ncells, int ngroups)
{

    nhost = pc->nhost();
    if (nhost < 2) { // for no PVM or MPI and for 1 host
        nhost = 1;
        myid = 0;
    } else {
        myid = pc->id();
    }
    nwork = nhost;
    ncell = ncells;
    ngroup = ngroups;
    ncellgrp = (int) ncells / ngroups;
    //cells = new List() // the worker cells
    //nclist = new List() // the netcons connecting to cells in this subset

    maxstepsize_called_ = 0;
    want_graph_ = 0;
    edgecount_ = 0;
    spikevec.resize(1000) ;
    idvec.resize(1000) ;
}


/*! 
 *  Master process calls ParNetwork2BBS's done() to terminate the MPI processors 
 *  
 */ 
void ParallelNetManager::done()
{
    if (myid == 0) pc->done();
}

/**
 * set_gid2node: originally
 * the gid <-> cell map was constructed in two phases.
 * first we specify which gids will exist on this machine.
 * Then, when create_cell is called we can decide if the cell
 * will actually be created and, if so, pc->presyn actually
 * creates the PreSyn, sets the gid, and makes the gid2PreSyn map.
 * that is deprecated.
 * Now it is best merely to call
 * register_cell(gid, cellobject) and that will both call gid_exists (if it
 * does not already exist), and make the mapping.
 *
 * @param cell_id global cell ID
 * @param pcid rank of node 
 */
void ParallelNetManager::set_gid2node(int cell_id, int pcid = -1)
{
    if (pcid == -1) pcid = myid; //default to myid
    pc->set_gid2node(cell_id, pcid);
#if DEBUG ==2
    std::cout << "Cell " << cell_id << " set by me to host " << pcid << std::endl;
#endif
}


//!simplistic partitioning of neurons on nodes 
void ParallelNetManager::load_balance_round_robin()   
{
    for (register int i = 0; i < ncell; ++i) {
        set_gid2node(i, i % nwork);
    }
    cell_cnt = 0;
}

//! order partitioning based on num of nodes
void ParallelNetManager::load_balance_roulette()  
{
    for (register int i = 0; i < ncell; ++i) {
        set_gid2node(i, floor(i*nwork / ncell));
    }
    cell_cnt = 0;
}

//! partition neurons based on groups
void ParallelNetManager::load_balance_by_group()   
{
    if (ngroup > nwork) {
        for (register int gr = 0; gr < ngroup; ++gr) {
            for (register int nc = 0; nc < ncellgrp; ++nc)
                set_gid2node(nc + gr*ncellgrp, gr % nwork);
        }
    } else {
        for (register int nc = 0; nc < ncell; ++nc)
            set_gid2node(nc, floor(nc*nwork / ncell));
    }
    cell_cnt = 0;
}

//! Check to see if global ID exists on this node
bool ParallelNetManager::gid_exists(int cell_id)
{
    return pc->gid_exists(cell_id);
}

/*
void  ParallelNetManager::want_all_spikes() {
    for(register int i=0; i<ncell;++i) {
        spike_record(i);
    }
}

void ParallelNetManager::spike_record(int cell_id) {
    if (gid_exists(cell_id)) {
        pc->spike_record(cell_id, spikevec, idvec);
    }
}
*/

 
/*! 
 *  creates a cell such as "new Cell(x, y, z)" in original NEURON
 * 
 * @param cell_id global ID
 * @param gr pointer to Group
 */
void ParallelNetManager::create_cell(int cell_id, Group * gr)
{
    if (gid_exists(cell_id)) {
        register_cell(cell_id, gr);
    }

}

//! Create cell on this node and register a syn connection
void ParallelNetManager::register_cell(int cell_id, Group* gr) /**< Create the neuron on this node */
{
//TODO  ConfigBase * nc;
    if (!pc->gid_exists(cell_id)) {
        pc->set_gid2node(cell_id, myid);
    }
    // all existing cells must have an associated gid which
    // is stored in the cell's PreSyn. The nc below will be
    // unreffed but the PreSyn will continue
    // in existence and from the gid we will quickly be able
    // to find the PreSyn and from that the Cell
    // we force the cell to be an outputcell due to the danger of
    // user error
//TODO  cells.push_back(cell_cnt++,cell);
//TODO  nc = new SynapseInterface(cell, nil);
//TODO  pc->cell(cell_id, nc, 1);
}

void ParallelNetManager::synmech_append(int precell_id, int postcell_id)
{
    int w, se, ww, i = -1;
    if (gid_exists(postcell_id)) {
        // target in this subset
        // source may be on this or another machine
//TODO      nc = cm2t(precell_id, pc->gid2cell(postcell_id), threshold, weight, delay)
//TODO      i = synlist.size();
//TODO      synlist.push_back(nc);
    } else if ((se = gid_exists(precell_id)) > 0) {
        // source exists but not the target
        if (se != 3) { // output to another machine and it is
            // not yet an outputcell
            pc->outputcell(precell_id);
        }
    }

}

ConfigBase* ParallelNetManager::cm2t(int precell_id, ConfigBase* postcell_syn, double weight, double delay)
{
    if (postcell_syn) {
        nc = pc->gid_connect(precell_id, postcell_syn);
    } /*else{
        nc = pc->gid_connect($1, $o2.synlist.object($3))
    } */
//  nc->weight = weight;
//  nc->delay = delay;
    return nc;
}


void ParallelNetManager::set_maxstep()
{
    // arg is max allowed, return val is just for this subnet
    localmaxstep_ = pc->set_maxstep(10); // arg is the maximum allowed
//  printf("%d localmaxstep=%g\n", myid, localmaxstep_)
}

void ParallelNetManager::maxstepsize()
{
    std::string s;
    if (!maxstepsize_called_) {
        maxstepsize_called_ = 1;
        if (nwork > 1) {
            bbsbuf.clear();
            append_string(s = "ParallelNetManager");
            append_string(s = "set_maxstep");
            pc->context();//this, "set_maxstep");
        }
        set_maxstep();
    }
}

// a safe way to get output sequentially on a per host basis
// without using the bulletin board. A file should be opened
// with File.aopen for appending at the beginning of the iterator_statement
// and closed at the end.
/*iterator serialize() {
    int rank
    pc->barrier()
    for (rank = 0; rank < pc->nhost; ++rank) {
        if (rank == pc->id) {
            iterator_statement;
        }
        pc->barrier();
    }
}

*/

 
/** doinit - prepare for simulation 
  * Need to emulate the tasks in SpikeSim and NEURON's
  * stdinit
  *    reinit_random_before_sim calls RandomGen members
  * 
  * @return 
  */
void ParallelNetManager::doinit()
{
    // reinitialisation of the random number generator
    if (SimEnv::reinit_random_before_sim())
        RandomGenerator::reinit();

    //prepare for spike_exchange
    
    
  //stdinit();
  // \ -setdt()
  //   -init()
  //   \ finitialise(vinit)
 
}


void ParallelNetManager::pinit()
{	      /**< Call workers to initialise simulation */
    maxstepsize();

   if (nwork > 1) {
        append_string("ParallelNetManager");
        append_string("doinit");
        pc->context();
    }
    doinit(); // the master does one also

}


//! See launch_sim
void ParallelNetManager::psolve(double x)
{

  launch_sim();

  //pc->psolve(x);  // this has to be done by PNM rather than BBS

  //integrate all parts

  //spike exchange

  //plastic update


}

void ParallelNetManager::pcontinue(double x)
{

    if (nwork > 1) {
        bbsbuf.clear();
        append_string("ParallelNetManager");
        append_string("psolve");
        append_double(x);
        pc->context();
    }
    psolve(x);
}

void ParallelNetManager::prun()
{
    pinit();
    pcontinue(tstop);
}


void ParallelNetManager::postwait(int x)
{
    double w;
    int sm, s, r, ru;
    if (x == 0) {
        bbsbuf.clear();
        append_int(myid);
        append_double(pc->wait_time());
        pc->post("waittime");//, myid, pc->wait_time());
    } else {
        w = pc->wait_time();
        sm = pc->spike_stat(&s, &sm, &r, &ru);
        bbsbuf.clear();
        append_int(myid);
        append_double(w);
        append_int(sm);
        append_int(s);
        append_int(r); append_int(ru);
        pc->post("poststat");//, myid, w, sm, s, r, ru);
    }
}
/*  HOC-like code here 
proc ParallelNetManager::prstat() { local i, id, w, sm, s, r, ru // print the wait time and statistics
    if (nwork > 1) {
        pc->context(this, "postwait", $1)
    }
    postwait($1)
    if ($1 == 0) {
        for i = 0, nwork - 1 {
            pc->take("waittime", &id, &w)
            printf("%d wait time %g\n", id, w)
        }
    }else{
        printf("id\t nsmax\t nsend\t nrecv\t nrused\t wait\n")
        for i = 0, nwork - 1 {
            pc->take("poststat", &id, &w, &sm, &s, &r, &ru)
            printf("%d\t %d\t %d\t %d\t %d\t %g\n", id, sm,s,r,ru,w)
        }
        printf("end of prstat\n")
    }
}

proc ParallelNetManager::postspikes() {
    pc->post("postspike", spikevec, idvec)
}

proc ParallelNetManager::gatherspikes() {local i  localobj s, id
    if (nwork > 1) {
        s = new Vector()
        id = new Vector()
        pc->context(this, "postspikes")
        for i=0, nwork-2 {
            pc->take("postspike", s, id)
            spikevec.append(s)
            idvec.append(id)
        }
    }
}

proc ParallelNetManager::wantgraph() {
    want_graph_ = 1
}

// metis graph partitioning input file has nnode+1 line format
// nnode  nedge 11
// nodes range from 1-nnode and there are nnode lines of form
// computationcost adjacentnode adjacentnodeweight adjacentnode w ...
// where computationcost must be an integer > 0 and we use adjacentnodeweight
// of 1000/mindelay.
// Although the graph is undirected I do not know if weights must be
// symetric but we force that. I do not know if
// node weight and edge weight is independent and unrelated to partitioning.

proc ParallelNetManager::graphout() {local i, j, jx, x  localobj f, cw
    if (!want_graph_) {
        printf("%s.wantgraph() was not called before building\n", this)
        return
    }
    f = new File($s1)
    f.wopen()
    f.printf("%d %d 11\n", cells.count, edgecount_)
    cellweight(cw)
    for i=0, cells.count-1 {
        f.printf("%d", cw.x[i])
        for jx=0, wmat_.sprowlen(i)-1 {
            x = wmat_.spgetrowval(i, jx, &j)
            f.printf(" %d %d", j+1, x)
        }
        f.printf("\n")
    }
    f.close()
}

proc ParallelNetManager::cellweight() {local i, act, loc
    $o1 = new Vector(cells.count)
    act = cvode_active()
    loc = cvode_local()
    if (!loc) {
        cvode_local(1)
    }
    stdinit()
    cvode.solve(.01)
    for i=0, cells.count-1 {
        $o1.x[i] = cells.object(i).cellweight(cvode)
    }
$o1.mul(100000)
$o1.printf
    if (!act) {
        cvode_active(0)
    }else if (!loc) {
        cvode_active(1)
    }
}

*/



/*! 
 * Build the network using the types in each Group. The reason for this class to control 
 * network construction was to assert gid to neurons and have access to all cell lists
 * This method was performed within the original SpikeSim Network class by letting each Group take care of itself. 
 *For the parallel system creation of the cells must be done by PNM and ParNetwork so that we can check to see if the cell is intended to be created on this node
 * 
 * @param net ParNetwork 
 */
void ParallelNetManager::create_network(ParNetwork& net)
{

    for (ParNetwork::ListGroupType::const_iterator grp = net.gp_list_.begin();
            grp != net.gp_list_.end();
	 ++grp){
      //(*i)->create_population();    
    //Create Neurons
      if ((!(*grp)->neuronconfigurator()) || (!(*grp)->dataconfigurator()))
        throw ConfigError("Group: void group or neuron configurator");
    else {
#ifdef DEBUG
        std::cout << "Creating cells in Group " << std::endl;
#endif
        NeuronFactory nrnfactory( (*grp)->dataconfigurator(),  (*grp)->neuronconfigurator());
        for (Size ii = 0; ii < (*grp)->size(); ++ii){
	  //Is this cell to be created on the current node
             if (gid_exists(cell_cnt++)) {
	       (*grp)->list_.push_back(boost::shared_ptr<NeuronInterface>(nrnfactory.create()));
	     }
	}
#ifdef DEBUG
        std::cout << "Completed creating cells in Group:"<< (*grp)->id << " size = " << (*grp)->list_.size() << std::endl;
#endif
    }
    }
    net.build_cell_list();  //inline function
}


/*! 
 * Connect groups of neurons based on the  
 * Conn class list in PNM.  The ParallelNetManager pointer to this class is passed to the 
 * source group in the Connectivity class.
 *
 * Output the num of connections in debug mode.
 *
 * @param net 
 */
void ParallelNetManager::connect_network(ParNetwork&net)
{
//------
//  net.connect_groups();
//------

      Size nb_con=0;
        for (ParNetwork::ListConnType::const_iterator i = net.conn_list_.begin();
             i != net.conn_list_.end();
             ++i){
    // connect the groups
	  (*i)->gp_source->par_connect_to(this,
		*(*i)->gp_target,
		(*i)->weight_distrib_cfg_.get(),
                (*i)->delay_distrib_cfg_.get(),
                (*i)->syn_mech_cfg_.get(),
                (*i)->plast_mech_cfg_.get(),
                (*i)->connectivity_cfg_.get(),
                net.cfg_list_,
                nb_con);
	    
	       // output on screen: write out the size of the constructed group
#ifdef DEBUG
                std::cout << nb_con << " connections from group #" << (*i)->gp_source->id << " to group #" << (*i)->gp_target->id  << std::endl;
#endif
            nb_con=0;
        }
    
}



void ParallelNetManager::launch_sim(ParNetwork & net)
{
  net.spike_exchange_init();
    while (SimEnv::i_time() < SimEnv::i_duration()) {
        // input updates
        ManageableInputManager::input_update_general();

        // output recordings to files and cleaning of the spike_lists
        if ((SimEnv::i_time() % (OutputManager::i_outputting_period())) == 0) {
            // display the simulated time on the console
            std::cout << SimEnv::sim_time() << std::endl;
            // performs the recurrent outputting
            OutputManager::do_output("during_sim");
            // clean the past spike history of the record neurons
            OutputManager::clear_past_of_spike_lists(net);
        }

        // performs the outputting at each time step
        OutputManager::do_output("each_time_step");

        // activation update of all the neurons (they call the update of the synapses)
        net.update();

	//Exchange Spikes
	//net.spike_exchange();
	
        // weight updates of the concerned plastic synapses (with the class DataPlastNeuron)
        if (SimEnv::sim_time() >= SimEnv::plasticity_effective_start_time())
            PlasticityManager::plast_update_general(); // only updates plastic neurons

        // advance the simulated time a timestep further
        SimEnv::advance();
    }
}
