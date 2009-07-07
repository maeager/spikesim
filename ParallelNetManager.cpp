



//#include "NetPar.h"
#include <errno.h>
#include "Group.h"
#include "GlobalDefs.h"
#include "AnyBuf.h"
#include "ParallelNetManager.h"
#include "ParNetwork2BBS.h"

#define nil 0

int ParallelNetManager::cell_cnt=0;


/*ParallelNetManager::ParallelNetManager()
{
}
*/

#ifdef CPPMPI
	ParallelNetManager::ParallelNetManager(int& argc,char**&argv)
#else
	ParallelNetManager::ParallelNetManager(int* argc,char***argv)
#endif
{
	pc = new ParNetwork2BBS(argc,argv);
	init(1,1);
	
}

ParallelNetManager::~ParallelNetManager()
{
}


void ParallelNetManager::init(int ncells, int ngroups) {
	
	nhost = pc->nhost();
	if (nhost < 2) { // for no PVM or MPI and for 1 host
		nhost = 1;
		myid = 0;
	}else{
		myid = pc->id();
	}
	nwork = nhost;
	ncell = ncells;
	ngroup = ngroups;
	ncellgrp = (int) ncells / ngroups;
	//cells = new List() // the worker cells
	//nclist = new List() // the netcons connecting to cells in this subset

	maxstepsize_called_ = 0;
	want_graph_ = 0;
	edgecount_ = 0;
	spikevec.resize(1000) ;
	idvec.resize(1000) ;
}


void ParallelNetManager::terminate() {
 	if( myid == 0) pc->done();
}

// originally
// the gid <-> cell map was constructed in two phases.
// first we specify which gids will exist on this machine.
// Then, when create_cell is called we can decide if the cell
// will actually be created and, if so, pc->presyn actually
// creates the PreSyn, sets the gid, and makes the gid2PreSyn map.
// that is deprecated.
// Now it is best merely to call
// register_cell(gid, cellobject) and that will both call gid_exists (if it
// does not already exist), and make the mapping.

void ParallelNetManager::set_gid2node(int cell_id, int pcid=-1) {
	if (pcid==-1) pcid = myid; //default to myid, master generally calls this.
	pc->set_gid2node(cell_id, pcid);
#ifdef DEBUG
	std::cout << "Cell " << cell_id << " set by me " << myid << std::endl;
#endif
}

void ParallelNetManager::load_balance_round_robin() { // simplistic partitioning
	for(register int i=0; i<ncell;++i) {
		set_gid2node(i, i%nwork);
	}
}

void ParallelNetManager::load_balance_roulette() { // in order partitioning
	for(register int i=0; i<ncell;++i) {
			set_gid2node(i, floor(i*nwork/ncell));
	}
}

void ParallelNetManager::load_balance_by_group() { // group partitioning
	if ( ngroup > nwork){
	for(register int gr=0; gr<ngroup;++gr) {
		for (register int nc=0;nc<ncellgrp;++nc)	
			set_gid2node(nc + gr*ncellgrp, gr%nwork);
	}}
	else {
		for (register int nc=0;nc<ncell;++nc)	
			set_gid2node(nc, floor(nc*nwork/ncell));
	}
}

bool ParallelNetManager::gid_exists(int cell_id) {
	return pc->gid_exists(cell_id);
}
/*
void  ParallelNetManager::want_all_spikes() {
	for(register int i=0; i<ncell;++i) {
		spike_record(i);
	}
}

void ParallelNetManager::spike_record(int cell_id) {
	if (gid_exists(cell_id)) {
		pc->spike_record(cell_id, spikevec, idvec);
	}
}
*/
// arg is gid and string that creates a cell such as "new Cell(x, y, z)"
// return the cell object (usually nil)
// this is deprecated
void ParallelNetManager::create_cell(int cell_id, Group * gr) {
	if (gid_exists(cell_id)) {
		register_cell(cell_id, gr);
	}

}

void ParallelNetManager::register_cell(int cell_id, Group* gr) { 
//TODO	ConfigBase * nc;
	if (!pc->gid_exists(cell_id)) { pc->set_gid2node(cell_id, myid); }
	// all existing cells must have an associated gid which
	// is stored in the cell's PreSyn. The nc below will be
	// unreffed but the PreSyn will continue
	// in existence and from the gid we will quickly be able
	// to find the PreSyn and from that the Cell
	// we force the cell to be an outputcell due to the danger of
	// user error
//TODO	cells.push_back(cell_cnt++,cell);
//TODO	nc = new SynapseInterface(cell, nil);
//TODO	pc->cell(cell_id, nc, 1);
}

void ParallelNetManager::synmech_append(int precell_id, int postcell_id) {
	int w,se,ww,i = -1;
	if (gid_exists(postcell_id)) {
		// target in this subset
		// source may be on this or another machine
//TODO		nc = cm2t(precell_id, pc->gid2cell(postcell_id), threshold, weight, delay)
//TODO		i = synlist.size();
//TODO		synlist.push_back(nc);
	} else if ((se = gid_exists(precell_id)) > 0) {
		// source exists but not the target
		if (se != 3){ // output to another machine and it is
			// not yet an outputcell
			pc->outputcell(precell_id);
		}
	}

}

ConfigBase* ParallelNetManager::cm2t(int precell_id, ConfigBase* postcell_syn, double weight, double delay) {
	if (postcell_syn) {
		nc = pc->gid_connect(precell_id, postcell_syn);
	} /*else{
		nc = pc->gid_connect($1, $o2.synlist.object($3))
	} */
//	nc->weight = weight;
//	nc->delay = delay;
	return nc;
}

void ParallelNetManager::set_maxstep() {
	// arg is max allowed, return val is just for this subnet
	localmaxstep_ = pc->set_maxstep(10); // arg is the maximum allowed
//	printf("%d localmaxstep=%g\n", myid, localmaxstep_)
}

void ParallelNetManager::maxstepsize() {
	int i, m;
	std::string s;
	if (!maxstepsize_called_) {
		maxstepsize_called_ = 1;
		if (nwork > 1) {
			bbsbuf.clear();
			append_string(s="ParallelNetManager");
			append_string(s="set_maxstep");
			pc->context();//this, "set_maxstep");
		}
		set_maxstep();
	}
}

// a safe way to get output sequentially on a per host basis
// without using the bulletin board. A file should be opened
// with File.aopen for appending at the beginning of the iterator_statement
// and closed at the end.
/*iterator serialize() {
	int rank
	pc->barrier()
	for (rank = 0; rank < pc->nhost; ++rank) {
		if (rank == pc->id) {
			iterator_statement;
		}
		pc->barrier();
	}
}
*/

void ParallelNetManager::doinit() {
	//stdinit();
}

void ParallelNetManager::pinit() {
	maxstepsize();
	if (nwork > 1) {
		append_string("ParallelNetManager");
		append_string("doinit");
	        pc->context();
	}
	doinit(); // the master does one also
}

void ParallelNetManager::psolve(double x) {
	pc->psolve(x);
}

void ParallelNetManager::pcontinue(double x) {
	
	if (nwork > 1) {
		bbsbuf.clear();
		append_string("ParallelNetManager");
		append_string("psolve");
		append_double(x);
		pc->context();
	}
	psolve(x);
}

void ParallelNetManager::prun() {
	pinit();
	pcontinue(tstop);
}

void ParallelNetManager::postwait(int x) {
	double w;
	int sm, s, r, ru;
	if (x == 0) {
		bbsbuf.clear();
		append_int(myid);
		append_double(pc->wait_time());
		pc->post("waittime");//, myid, pc->wait_time());
	}else{
		w = pc->wait_time();
		sm = pc->spike_stat(&s,&sm, &r, &ru);
		bbsbuf.clear();
		append_int(myid);
		append_double(w);
		append_int(sm);
		append_int(s);
		append_int(r); append_int(ru);
		pc->post("poststat");//, myid, w, sm, s, r, ru);
	}
}
/*
proc ParallelNetManager::prstat() { local i, id, w, sm, s, r, ru // print the wait time and statistics
	if (nwork > 1) {
		pc->context(this, "postwait", $1)
	}
	postwait($1)
	if ($1 == 0) {
		for i = 0, nwork - 1 {
			pc->take("waittime", &id, &w)
			printf("%d wait time %g\n", id, w)
		}
	}else{
		printf("id\t nsmax\t nsend\t nrecv\t nrused\t wait\n")
		for i = 0, nwork - 1 {
			pc->take("poststat", &id, &w, &sm, &s, &r, &ru)
			printf("%d\t %d\t %d\t %d\t %d\t %g\n", id, sm,s,r,ru,w)
		}
		printf("end of prstat\n")
	}
}

proc ParallelNetManager::postspikes() {
	pc->post("postspike", spikevec, idvec)
}

proc ParallelNetManager::gatherspikes() {local i  localobj s, id
	if (nwork > 1) {
		s = new Vector()
		id = new Vector()
		pc->context(this, "postspikes")
		for i=0, nwork-2 {
			pc->take("postspike", s, id)
			spikevec.append(s)
			idvec.append(id)
		}
	}
}

proc ParallelNetManager::wantgraph() {
	want_graph_ = 1
}

// metis graph partitioning input file has nnode+1 line format
// nnode  nedge 11
// nodes range from 1-nnode and there are nnode lines of form
// computationcost adjacentnode adjacentnodeweight adjacentnode w ...
// where computationcost must be an integer > 0 and we use adjacentnodeweight
// of 1000/mindelay.
// Although the graph is undirected I do not know if weights must be
// symetric but we force that. I do not know if
// node weight and edge weight is independent and unrelated to partitioning.

proc ParallelNetManager::graphout() {local i, j, jx, x  localobj f, cw
	if (!want_graph_) {
		printf("%s.wantgraph() was not called before building\n", this)
		return
	}
	f = new File($s1)
	f.wopen()
	f.printf("%d %d 11\n", cells.count, edgecount_)
	cellweight(cw)
	for i=0, cells.count-1 {
		f.printf("%d", cw.x[i])
		for jx=0, wmat_.sprowlen(i)-1 {
			x = wmat_.spgetrowval(i, jx, &j)
			f.printf(" %d %d", j+1, x)
		}
		f.printf("\n")
	}
	f.close()
}

proc ParallelNetManager::cellweight() {local i, act, loc
	$o1 = new Vector(cells.count)
	act = cvode_active()
	loc = cvode_local()
	if (!loc) {
		cvode_local(1)
	}
	stdinit()
	cvode.solve(.01)
	for i=0, cells.count-1 {
		$o1.x[i] = cells.object(i).cellweight(cvode)
	}
$o1.mul(100000)
$o1.printf
	if (!act) {
		cvode_active(0)
	}else if (!loc) {
		cvode_active(1)
	}
}

*/