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
	
	ParallelNetManager pnm(&argc,&argv);

	// TEST ParallelNetManager
	pnm.init(100,5); 
	if (pnm.myid == 0) {
	std::cout << "ncell = " << pnm.ncell << std::endl;
	std::cout << "nhost = " << pnm.nhost << std::endl;
	std::cout << "ngroup = " << pnm.ngroup << std::endl;
	std::cout << "ncellgrp = " << pnm.ncellgrp << std::endl;
	}
       	// TESTING load balances 
	pnm.load_balance_roulette();pnm.pc->gid_clear();pnm.pc->barrier(); if( pnm.myid == 0) std::cin.get();
	pnm.load_balance_round_robin();pnm.pc->gid_clear();pnm.pc->barrier(); if( pnm.myid == 0) std::cin.get();
	pnm.load_balance_by_group();pnm.pc->gid_clear();
	

	
	pnm.pc->barrier(); 
	
	if(pnm.myid == 0) {
	  std::cout << "Hit Enter to finish" << std::endl;
	  std::cin.get();
	  pnm.done();
	} else 	MPI_Finalize();
	
	return 0;
}
