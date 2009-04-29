#pragma once

#include "ParSpike.h"
#include "nrnhash.h"
#include "BBS.h"


#undef MD
#define MD 2147483648.

class PreSyn;

// hash table where buckets are binary search maps
declareNrnHash(Gid2PreSyn, int, PreSyn*)
implementNrnHash(Gid2PreSyn, int, PreSyn*)

extern NetCvode* net_cvode_instance;
extern double t, dt;
extern int cvode_active_;
extern Point_process* ob2pntproc(Object*);
extern int nrn_use_selfqueue_;
extern void nrn_pending_selfqueue(double, NrnThread*);

extern void ncs2nrn_integrate(double tstop);
extern void nrn_fake_fire(int gid, double firetime, int fake_out);
extern void nrn_partrans_clear();
extern int nrnmpi_int_allmax(int);
extern void nrnmpi_int_allgather(int*, int*, int);

class NetPar 
{

public:
	NetPar(void);

	void alloc_space();
	int spike_compress(int nspike, boolean gid_compress, int xchng_meth);
	void spike_exchange_compressed();	
	void gid_clear();
	void spike_exchange_init();
	double set_mindelay(double maxdelay);
	void timeout(int);
	void spike_exchange();
	void nrn2ncs_outputevent(int netcon_output_index, double firetime);
	void nrn_outputevent(unsigned char localgid, double firetime);

	 Symbol* netcon_sym_;
	 Gid2PreSyn* gid2out_;
	 Gid2PreSyn* gid2in_;
	 double t_exchange_;
	 double dt1_; // 1/dt

// for compressed gid info during spike exchange
	bool nrn_use_localgid_;

	static Gid2PreSyn** localmaps_;

#define NRNSTAT 1
	 int nsend_, nsendmax_, nrecv_, nrecv_useful_;
#if NRNSTAT
	 std::vector<float> max_histogram_;
#endif 

	 int ocapacity_; // for spikeout_
	// require it to be smaller than  min_interprocessor_delay.
	 double wt_; // wait time for nrnmpi_spike_exchange
	 double wt1_; // time to find the PreSyns and send the spikes.
	 boole use_compress_;
	 int spfixout_capacity_;
	 int idxout_;

	 int active_;
	 double usable_mindelay_;
	 double min_interprocessor_delay_;	
	 double mindelay_; // the one actually used. Some of our optional algorithms
	 double last_maxstep_arg_;
	 NetParEvent* npe_; // nrn_nthread of them
	 int n_npe_; // just to compare with nrn_nthread

private:

inline static void sppk(unsigned char* c, int gid) {
	for (int i = localgid_size_-1; i >= 0; --i) {
		c[i] = gid & 255;
		gid >>= 8;
	}
}
inline static int spupk(unsigned char* c) {
	int gid = *c++;
	for (int i = 1; i < localgid_size_; ++i) {
		gid <<= 8;
		gid += *c++;
	}
	return gid;
}
	int localgid_size_;
};


