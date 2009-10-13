


include ../Makefile.343
# include ../Makefile.eee
# include ../Makefile.soma


test_pnm.o: test_pnm.cpp  ../ParNetwork.h ../BBS.h ../ParSpike.h ../BBServer.h ../BBS2MPI.h ../ParNetwork2BBS.h ../ParallelNetManager.h
	$(MPICXX) $(CXXFLAGS)  -c test_pnm.cpp -o test_pnm.o

test_pnm: test_pnm.o $(ParSpikeSim_CXXOBJECTS)
	$(MPICXX) $(CXXFLAGS) -o test_pnm test_pnm.o $(ParSpikeSim_CXXOBJECTS)


load_balances.o: load_balances.cpp  ../ParNetwork.h ../BBS.h ../ParSpike.h ../BBServer.h ../BBS2MPI.h ../ParNetwork2BBS.h ../ParallelNetManager.h
	$(MPICXX) $(CXXFLAGS) -I..  -c load_balances.cpp -o load_balances.o

load_balances: load_balances.o $(ParSpikeSim_OBJECTS)
	cd ..; $(MPICXX) $(CXXFLAGS) -o test/load_balances test/load_balances.o $(ParSpikeSim_CXXOBJECTS)
