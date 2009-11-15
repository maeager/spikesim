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

int
main(int argc, char *argv[])
{

    Size ncells = 0, ngroups = 0;
#ifdef CPPMPI
    ParallelNetManager pnm(argc, argv);
#else
    ParallelNetManager pnm(&argc, &argv);
#endif

    ParNetwork net;
    // TEST ParallelNetManager
    pnm.init(100, 5);
    if (pnm.myid == 0) {
        std::cout << "ncell = " << pnm.ncell << std::endl;
        std::cout << "nhost = " << pnm.nhost << std::endl;
        std::cout << "ngroup = " << pnm.ngroup << std::endl;
        std::cout << "ncellgrp = " << pnm.ncellgrp << std::endl;
    }


    std::cout << "[info] press any key at end of execution to close this window" << std::endl << std::endl;
    // construction of the network, initialisation of the simulation environment, etc.
    try {
        net.config_from_file("./script.txt", ncells, ngroups);
    } catch (ConfigError & err) {
        std::cout << err.what();
        // terminates the execution
        std::cin.get();
        return EXIT_FAILURE;
    } catch (...) {
        std::cout << "ParNetwork: unknown error when building from script file";
        // terminates the execution
        std::cin.get();
        return EXIT_FAILURE;
    }

    pnm.pc->barrier();
    if (pnm.myid == 0) {
        std::cout << "ncells " << ncells << ", ngroups " << ngroups << std::endl;
        std::cout << "Hit Enter to continue" << std::endl;
        std::cin.get();
    }

    pnm.init(ncells, ngroups);
    if (pnm.myid == 0) {
        std::cout << "ncell = " << pnm.ncell << std::endl;
        std::cout << "nhost = " << pnm.nhost << std::endl;
        std::cout << "ngroup = " << pnm.ngroup << std::endl;
        std::cout << "ncellgrp = " << pnm.ncellgrp << std::endl;
    }

    pnm.load_balance_roulette();
    pnm.create_network(net);

//Round robin is probably the most inefficient way to distribute the neurons
//Guy from IBM said that holding all the synapse on CPUs would be better


    //std::cout << std::endl << "ParNetwork size " << net.network_size() << std::endl;

    if (pnm.myid == 0) std::cout << "simulation duration: " << (SimEnv::i_duration() * SimEnv::timestep()) << std::endl;
    if (pnm.myid == 0) std::cout << std::endl << "[info] press any key now to start the execution" << std::endl << std::endl;
 //   std::cin.get();

    // open the output files
    OutputManager::open_files();
    OutputManager::do_output_connectivity(std::list<Size>(), std::list<Size>(), 1);


    // start time clock
    long actual_start_time = (long) time(NULL);

    // reinitialisation of the random number generator
    if (SimEnv::reinit_random_before_sim())
        RandomGenerator::reinit();

    // start of the simulation



    // stop time clock
    long actual_stop_time = (long) time(NULL);
    if (pnm.myid == 0) std::cout << "real-time duration of the simulation: " << (actual_stop_time - actual_start_time) << " seconds" << std::endl;

    // additional outputs
    OutputManager::do_output("end_sim");
    OutputManager::close_all();

    // memory cleaning

    std::cout << "Hello World! I am " << pnm.myid << " of " << pnm.nhost << std::endl;

    //pnm.pc->barrier();
    //if (pnm.myid == 0) {
    //    std::cin.get(); }
    pnm.terminate();
    // wait for key pressed
    return 0;
}
