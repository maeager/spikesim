#include <cstdlib>
#include <iostream>

#include <fstream>
#include <time.h>
#ifndef CPPMPI
#define CPPMPI
#endif

#include "TypeDefs.h"
#include "SimulationEnvironment.h"
#include "OutputManager.h"
#include "ParallelNetManager.h"
#include "ParNetwork.h"
#include <mpi.h>

int 
main(int argc, char *argv[])
{
	
	Size ncells=0,ngroups=0;
	ParallelNetManager pnm(argc,argv);
	ParNetwork net;

	// construction of the network, initialisation of the simulation environment, etc.
	try {
		net.config_from_file("./script.txt",ncells,ngroups);
	pnm.init(ncells,ngroups);
	pnm.load_balance_roulette();
	pnm.create_network(net);

//Round robin is probably the most inefficient way to distribute the neurons
//Guy from IBM said that holding all the synapse on CPUs would be better
	

	std::cout << std::endl << "ParNetwork size " << net.network_size() << std::endl;


	} catch (ConfigError & err) {
		std::cout << err.what();
		// terminates the execution
		
		std::cin.get();
		return EXIT_FAILURE;
	} catch(...) {
		std::cout << "ParNetwork: unknown error when building from script file";
		// terminates the execution
		std::cin.get();
		return EXIT_FAILURE;
	}
	/*testing*/ pnm.pc->barrier(); if( pnm.myid == 0) {std::cout << "Hit Enter to continue" << std::endl; std::cin.get();}




	std::cout << "simulation duration: " << (SimEnv::i_duration() * SimEnv::timestep()) << std::endl;
	
	
	pnm.pc->barrier(); 
	if(pnm.myid == 0) {pnm.terminate();}
	
	// wait for key pressed
	return 0;
}
