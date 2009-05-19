
#include "ParSpike.h"
#include "ParNetwork.h"
#include "NetPar.h"

class  ParallelNetManager
{
public:
	void register_cell();
	void nc_append();
	void set_gid2node();
	bool gid_exists();
	void create_cell();

	ParallelContext pc;
	int maxstepsize;
	void gatherspikes();
void want_all_spikes();
void spike_record(int);
void prun(), pcontinue(), pinit(), psolve(); 

void round_robin(); //simplistic partitioning


// mostly for debugging
	std::vector<double> spikevec;
	std::vector<double> idvec;
std::map<int,ConfigBase>  cells;
std::list<SynMechInterface>  synlist; 

int myid;
int ncell;
int nwork; 
int nhost;
int prstat;
SynMechInterface * nc;
//external stdinit, continuerun, cvode, tstop, hoc_sf_
//external cvode_active, cvode_local
}
