
class NetCon;
class NeuroBase;
class SynPtr; 


class  ParallelNetManager
{
public:
	void register_cell();
	void nc_append();
	void set_gid2node()
	bool gid_exists();
	void create_cell()

	ParallelConfig pc;
	int maxstepsize
	double * spikevec;
	int * idvec;

void gatherspikes();
void want_all_spikes();
void spike_record(int);
public graphout, wantgraph, set_maxstep, serialize
void prun(), pcontinue(), pinit(), psolve(); 
int ncell
void round_robin(); //simplistic partitioning
void * tmpcell;
// mostly for debugging
	std::vector<double> spikevec;
	std::vector<double> idvec;
std::list<ConfigBase>  cells;
std::list<NetCon>  nclist; 
int myid;
int nwork; 
int nhost;
int prstat;
SynPtr * nc;
external stdinit, continuerun, cvode, tstop, hoc_sf_
external cvode_active, cvode_local
}
