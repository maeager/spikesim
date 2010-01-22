#include <cstdlib>
#include <iostream>

#include <fstream>
#include <time.h>

#include "TypeDefs.h"

#ifdef PARALLELSIM
#include "ParNetwork.h"
#include <mpi.h>
#else
#include "Network.h"
#endif

#include "SimulationEnvironment.h"
#include "OutputManager.h"
#include "NoThreading.h"
#include "Engine.h"



int main(int argc, char *argv[])
{
#ifdef PARALLELSIM
    MPI::Init(argc, argv);
    int rank = MPI::COMM_WORLD.Get_rank();
    int size = MPI::COMM_WORLD.Get_size();
    ParNetwork net;
#else
    Network net;
#endif
        std::cout << "[info] press any key at end of execution to close this window" << std::endl << std::endl;
        // construction of the network, initialisation of the simulation environment, etc.
        try {
            net.build_from_file("./script.txt");
        } catch (ConfigError & err) {
            std::cout << err.what();
            // terminates the execution
            std::cin.get();
            return EXIT_FAILURE;
        } catch (...) {
            std::cout << "Network: unknown error when building from script file";
            // terminates the execution
            std::cin.get();
            return EXIT_FAILURE;
        }
#ifdef PARALLELSIM
        //net.build_network();
        std::cout << std::endl << "ParNetwork size " << net.network_size() << std::endl;
#endif
        std::cout << "simulation duration: " << (SimEnv::i_duration() * SimEnv::timestep()) << std::endl;
        std::cout << std::endl << "[info] press any key now to start the execution" << std::endl << std::endl;
        std::cin.get();

        // open the output files
        OutputManager::open_files();

        OutputManager::do_output_connectivity(std::list<Size>(), std::list<Size>(), 1);


        // start time clock
        long actual_start_time = (long) time(NULL);

        // reinitialisation of the random number generator
        if (SimEnv::reinit_random_before_sim())
            RandomGenerator::reinit();

        // start of the simulation
        Engine<NoThreading> engine;

        engine.init(net);
        engine.launch_sim(net);


        // stop time clock
        long actual_stop_time = (long) time(NULL);
        std::cout << "real-time duration of the simulation: " << (actual_stop_time - actual_start_time) << " seconds" << std::endl;

        // additional outputs
        OutputManager::do_output("end_sim");
        OutputManager::close_all();

        // memory cleaning
#ifdef PARALLELSIM

    std::cout << "Hello World! I am " << rank << " of " << size <<
              std::endl;

    MPI::Finalize();
#endif

    // wait for key pressed
//  std::cin.get();
    return 0;
}
