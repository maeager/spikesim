#ifndef PARALLELMANAGER_H
#define PARALLELMANAGER_H

#include "ParSpike.h"

class  ParallelManager
{
public:
	ParallelManager(int n){init(n);}
	~ParallelManager(){if (pc.mpi_use) pc.terminate();}
	void register_cell();
	void nc_append();
	void set_gid2node()
	bool gid_exists();
	void create_cell();

	ParSpike *pc;
	int maxstepsize
	std::vector<float> spikevec;
	std::vector<int> idvec;

//	void gatherspikes();
//void want_all_spikes();
//void spike_record(int);
//public graphout, wantgraph, set_maxstep, serialize

	void prun(), pcontinue(), pinit(), psolve(); 
	int ncell
	void round_robin(); //simplistic partitioning
	void * tmpcell;
// mostly for debugging

	int myid;
	int nwork; 
	int nhost;
	int prstat;
	SynPtr * nc;
};


void init(int ncells) {
	pc = new ParSpike;
	nhost = pc.nhost;
	if (nhost < 2) { // for no PVM or MPI and for 1 host
		nhost = 1;
		myid = 0;
	}else{
		myid = pc.id;
	}
	nwork = nhost;
	ncell = ncells;
	
	//cells = new List() // the worker cells
	//nclist = new List() // the netcons connecting to cells in this subset

	maxstepsize_called_ = 0;
	want_graph_ = 0;
	edgecount_ = 0;
	spikevec.resize(0);
	idvec.resize(0);
}

// originally
// the gid <-> cell map was constructed in two phases.
// first we specify which gids will exist on this machine.
// Then, when create_cell is called we can decide if the cell
// will actually be created and, if so, pc.presyn actually
// creates the PreSyn, sets the gid, and makes the gid2PreSyn map.
// that is deprecated.
// Now it is best merely to call
// register_cell(gid, cellobject) and that will both call gid_exists (if it
// does not already exist), and make the mapping.

void set_gid2node(int cell_id) {
		pc.set_gid2node(cell_id, myid);
}

void round_robin() { // simplistic partitioning
	for(register int i=0; i<ncell;++i) {
		set_gid2node(i, i%nwork);
	}
}

bool gid_exists(int cell_id) {
	return pc.gid_exists(cell_id);
}

void  want_all_spikes() {local i
	for(register int i=0; i<ncell;++i) {
		spike_record(i);
	}
}

void spike_record(int cell_id) {
	if (gid_exists(cell_id)) {
		pc.spike_record(cell_id, spikevec, idvec);
	}
}

// arg is gid and string that creates a cell such as "new Cell(x, y, z)"
// return the cell object (usually nil)
// this is deprecated
void create_cell(int cell_id, NeuronType * nt) {
	if (gid_exists(cell_id)) {
		register_cell(cell_id, nt)
	}

}

void register_cell(int cell_id, void* cell) { 
	void * nc;
	if (!pc.gid_exists($1)) { pc.set_gid2node(cell_id, pc.id); }
	// all existing cells must have an associated gid which
	// is stored in the cell's PreSyn. The nc below will be
	// unreffed but the PreSyn will continue
	// in existence and from the gid we will quickly be able
	// to find the PreSyn and from that the Cell
	// we force the cell to be an outputcell due to the danger of
	// user error
	cells.append(cell)
	nc = new NetCon(cell, nil)
	pc.cell(cell_id, nc, 1);
}

void nc_append(int precell_id, int postcell_id) {
	int w,se,ww,i = -1;
	if (gid_exists(postcell_id)) {
		// target in this subset
		// source may be on this or another machine
		nc = cm2t(precell_id, pc.gid2cell(postcell_id), $3, $4, $5)
		i = nclist.count
		nclist.append(nc)
	} else if ((se = gid_exists(precell_id)) > 0) {
		// source exists but not the target
		if (se != 3){ // output to another machine and it is
			// not yet an outputcell
			pc.outputcell(precell_id);
		}
	}

}

obfunc cm2t(int precell_id, int postcell_id) {
	if ($3 < 0) {
		nc = pc.gid_connect($1, $o2)
	}else{
		nc = pc.gid_connect($1, $o2.synlist.object($3))
	}
	nc.weight = $4
	nc.delay = $5
	return nc
}

void set_maxstep() {
	// arg is max allowed, return val is just for this subnet
	localmaxstep_ = pc.set_maxstep(10); // arg is the maximum allowed
//	printf("%d localmaxstep=%g\n", myid, localmaxstep_)
}

proc maxstepsize() {local i, m
	if (!maxstepsize_called_) {
		maxstepsize_called_ = 1
		if (nwork > 1) {
			pc.context(this, "set_maxstep")
		}
		set_maxstep()
	}
}

// a safe way to get output sequentially on a per host basis
// without using the bulletin board. A file should be opened
// with File.aopen for appending at the beginning of the iterator_statement
// and closed at the end.
iterator serialize() {local rank
	pc.barrier
	for rank = 0, pc.nhost {
		if (rank == pc.id) {
			iterator_statement
		}
		pc.barrier
	}
}

proc doinit() {
	stdinit()
}

proc pinit() {
	maxstepsize()
	if (nwork > 1) {
	        pc.context(this, "doinit")
	}
	doinit() // the master does one also
}

proc psolve() {
	pc.psolve($1)
}

proc pcontinue() {
	if (nwork > 1) {
		pc.context(this, "psolve", $1)
	}
	psolve($1)
}

proc prun() {
	pinit()
	pcontinue(tstop)
}

proc postwait() {local w, sm, s, r, ru
	if ($1 == 0) {
		pc.post("waittime", myid, pc.wait_time())
	}else{
		w = pc.wait_time()
		sm = pc.spike_statistics(&s, &r, &ru)
		pc.post("poststat", myid, w, sm, s, r, ru)
	}
}

proc prstat() { local i, id, w, sm, s, r, ru // print the wait time and statis
	if (nwork > 1) {
		pc.context(this, "postwait", $1)
	}
	postwait($1)
	if ($1 == 0) {
		for i = 0, nwork - 1 {
			pc.take("waittime", &id, &w)
			printf("%d wait time %g\n", id, w)
		}
	}else{
		printf("id\t nsmax\t nsend\t nrecv\t nrused\t wait\n")
		for i = 0, nwork - 1 {
			pc.take("poststat", &id, &w, &sm, &s, &r, &ru)
			printf("%d\t %d\t %d\t %d\t %d\t %g\n", id, sm,s,r,ru,w)
		}
		printf("end of prstat\n")
	}
}

proc postspikes() {
	pc.post("postspike", spikevec, idvec);
}

proc gatherspikes() {local i  
	
	if (nwork > 1) {
		std::vector<float> s,id;
		pc.context(this, "postspikes")
		for i=0, nwork-2 {
			pc.take("postspike", s, id)
			spikevec.append(s);
			idvec.append(id);
		}
	}
}
#endif