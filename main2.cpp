#include <cstdlib>
#include <iostream>

#include <fstream>
#include <time.h>

#include "TypeDefs.h"


#include "ParNetwork.h"
#include <mpi.h>


#include "SimulationEnvironment.h"
#include "OutputManager.h"
#include "ParallelNetManager.h"



int 
main(int argc, char *argv[])
{
	
	ParallelNetManager pnm(&argc,&argv);
	ParNetwork net;
	if (pnm.my_rank==0){
	std::cout << "[info] press any key at end of execution to close this window" << std::endl << std::endl;
	// construction of the network, initialisation of the simulation environment, etc.
	try {
		net.build_from_file("./script.txt");
	} catch (ConfigError & err) {
		std::cout << err.what();
		// terminates the execution
		
		std::cin.get();
		return EXIT_FAILURE;
	} catch(...) {
		std::cout << "Network: unknown error when building from script file";
		// terminates the execution
		std::cin.get();
		return EXIT_FAILURE;
	}

	net.build_network();
	pnm.ncell = net.network_size();
	pnm.round_robin();

	std::cout << std::endl << "ParNetwork size " << net.network_size() << std::endl;

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



	// stop time clock
	long actual_stop_time = (long) time(NULL);
	std::cout << "real-time duration of the simulation: " << (actual_stop_time - actual_start_time) << " seconds" << std::endl;

	// additional outputs
	OutputManager::do_output("end_sim");
	OutputManager::close_all();
	}
	// memory cleaning

	std::cout << "Hello World! I am " << pnm.my_rank << " of " << size <<
	std::endl;
	
	pnm.terminate();

	// wait for key pressed
//	std::cin.get();
	return 0;
}
