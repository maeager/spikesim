#include <cstdlib>
#include <iostream>

#include <fstream>
#include <time.h>

#include "TypeDefs.h"

#include "SimulationEnvironment.h"
#include "OutputManager.h"
#include "ParallelNetManager.h"
#include "ParNetwork.h"
#include <mpi.h>

using namespace std;

int
main(int argc, char *argv[])
{

#ifdef CPPMPI
    ParallelNetManager pnm(argc, argv);
#else
    ParallelNetManager pnm(&argc, &argv);
#endif
    std::cout << "Hello World! I am " << pnm.myid << " of " << pnm.nhost << std::endl;
    cout << "ParSpikeSim: I am " << pnm.myid << " of " << pnm.nhost << endl;

    // TEST ParallelNetManager
    pnm.init(100, 5);
    if (pnm.myid == 0) {
        std::cout << "ncell = " << pnm.ncell << std::endl;
        std::cout << "nhost = " << pnm.nhost << std::endl;
        std::cout << "ngroup = " << pnm.ngroup << std::endl;
        std::cout << "ncellgrp = " << pnm.ncellgrp << std::endl;
    }
    // TESTING load balances
    pnm.load_balance_roulette(); pnm.pc->gid_clear(); pnm.pc->barrier(); if (pnm.myid == 0) std::cin.get();
    pnm.load_balance_round_robin(); pnm.pc->gid_clear(); pnm.pc->barrier(); if (pnm.myid == 0) std::cin.get();
    pnm.load_balance_by_group(); pnm.pc->gid_clear();

    //TESTING ParNetwork
    ParNetwork net;

    Size ncells = 0, ngroups = 0;

    std::cout << "[info] press any key at end of execution to close this window" << std::endl << std::endl;
    // construction of the network, initialisation of the simulation environment, etc.
    try {
        net.config_from_file("./script.txt", ncells, ngroups);
      //net.build_from_file("./script.txt");
	
    } catch (ConfigError & err) {
      cout <<"Error "<< err.what();
        // terminates the execution
	cin.get();
        return EXIT_FAILURE;
    } catch (...) {
        cout << "ParNetwork: unknown error when building from script file";
        // terminates the execution
        cin.get();
        return EXIT_FAILURE;
    }
    //End testing
     pnm.pc->barrier();
     if (pnm.myid == 0) {
        std::cout << "Hit Enter to continue" << std::endl; std::cin.get();
    }
     //    pnm.init(ncells, ngroups);
     //pnm.load_balance_round_robin();
    // Start Creation of network cells
    // net.create_population();

    //End Setup of ParNetwork
    if (pnm.myid != 0) {
      //      pnm.pc->barrier(); 
    } else {
        cout << "TESTING ParNetwork" << endl;
        cout << "cell_list size  = " <<   net.network_size() << endl;
        cout << "conn list size = " << net.conn_list_.size() << endl;  
        cout << "group list size= " << net.gp_list_.size() << endl;
        cout << "cfg list size= " << net.cfg_list_.size() << endl;
        cout << "presyn list size= " << net.presyn_list.size() << endl;
        cout << "postsyn list size= " << net.postsyn_list.size() << endl;
        cout << "Hit Enter to continue" << endl; //cin.get();
     }
     cout << "Ncells = " << ncells << "\tNgroups = " << ngroups << endl;
    pnm.init(ncells, ngroups);
//Round robin is probably the most inefficient way to distribute the neurons
//Guy from IBM said that holding all the synapse on CPUs would be better
     pnm.load_balance_round_robin();
     if (pnm.myid == 0) {
       cout << "TESTING ParallelNetManager" << endl;
       cout << "\tncell = " << pnm.ncell << endl;
       cout << "\tngroup = " << pnm.ngroup << endl;
       cout << "\tncellgrp = " << pnm.ncellgrp << endl;
     }


     // Start Creation of network cells
     pnm.create_network(net);
     pnm.connect_network(net);
     pnm.pc->barrier(); 
    if (pnm.myid == 0) {
      //      pnm.prun();
    
    }




/*    cout << "Simulation duration: " << (SimEnv::i_duration() * SimEnv::timestep()) << endl;
    cout << endl << "[info] press any key now to start the execution" << endl << endl;
    cin.get();

    // open the output files
    OutputManager::open_files();

    OutputManager::do_output_connectivity(list<Size>(), list<Size>(), 1);


    // start time clock
    long actual_start_time = (long) time(NULL);

    // reinitialisation of the random number generator
    if (SimEnv::reinit_random_before_sim())
        RandomGenerator::reinit();

    // start of the simulation



    // stop time clock
    long actual_stop_time = (long) time(NULL);
    if (pnm.myid == 0)  cout << "real-time duration of the simulation: " << (actual_stop_time - actual_start_time) << " seconds" << endl;

    // additional outputs
    OutputManager::do_output("end_sim");
    OutputManager::close_all();


    // memory cleaning
*/

    
	pnm.pc->barrier(); 
	
	if(pnm.myid == 0) {
	  std::cout << "Hit Enter to finish" << std::endl;
	  std::cin.get();
	  pnm.done();
	} else 	MPI_Finalize();



    return 0;
}
