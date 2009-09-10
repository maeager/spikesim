MPICXX = /opt/mpich/ch-p4/bin/mpicxx
CXXFLAGS = -Wall -I/usr/local -I/opt/mpich/ch-p4/include -DPARALLELSIM -DDEBUG  -DCCPMPI #
ParSpikeSim_OBJECTS= NetPar.o ../ParNetwork.o ../BBS.o ../ParSpike.o ../BBServer.o ../BBClient.o ../BBS2MPI.o ../ ../ParNetwork2BBS.o ../ParallelNetManager.o ../ ../CondSynMech.o ../ConnectivityManager.o ../DataIndivSynapse.o ../DataLightSynapse.o ../DataPlastNeuron.o ../DataPlastSynapse.o ../DeltaCorrInputRefs.o ../DistributionManager.o ../DoubleExpSynMech.o ../Group.o ../IFMech.o ../InstantIFMech.o ../InstantSynMech.o ../ManageableInput.o ../NeuronActivationFunction.o ../ ../OscillatoryCorrInputRefs.o ../OutputManager.o ../PoissonMech.o ../RandomGenerator.o ../STDPMech.o ../ShortTimeCorrInputRefs.o ../SimulationEnvironment.o 
ParSpikeSim_CXXOBJECTS= ../NetPar.o ../ParNetwork.o ../BBS.o ../ParSpike.2.o ../BBServer.o ../BBClient.o ../BBS2MPI.2.o ../ ../ParNetwork2BBS.o ../ParallelNetManager.o ../ ../CondSynMech.o ../ConnectivityManager.o ../DataIndivSynapse.o ../DataLightSynapse.o ../DataPlastNeuron.o ../DataPlastSynapse.o ../DeltaCorrInputRefs.o ../DistributionManager.o ../DoubleExpSynMech.o ../Group.o ../IFMech.o ../InstantIFMech.o ../InstantSynMech.o ../ManageableInput.o ../NeuronActivationFunction.o ../ ../OscillatoryCorrInputRefs.o ../OutputManager.o ../PoissonMech.o ../RandomGenerator.o ../STDPMech.o ../ShortTimeCorrInputRefs.o ../SimulationEnvironment.o 


test_pnm.o: test_pnm.cpp  ../ParNetwork.h ../BBS.h ../ParSpike.h ../BBServer.h ../BBS2MPI.h ../ParNetwork2BBS.h ../ParallelNetManager.h
	$(MPICXX) $(CXXFLAGS)  -c test_pnm.cpp -o test_pnm.o

test_pnm: test_pnm.o $(ParSpikeSim_CXXOBJECTS)
	$(MPICXX) $(LDFLAGS) $(CXXFLAGS) -o test_pnm test_pnm.o $(ParSpikeSim_CXXOBJECTS)
