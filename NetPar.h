#ifndef netpar_h
#define netpar_h

#include "nrnhash.h"
#include "ParSpike.h"
#include "ParNetwork.h"
#include "BBS.h"

#include <list>
#include <fstream>
#include <string>
#include<boost/shared_ptr.hpp>

#include "IdCounter.h"
#include "Error.h"
#include "DataCommonNeuron.h"
#include "DataRecordNeuron.h"
#include "TypeDefs.h"
#include "GlobalDefs.h"
#include "InterfaceBase.h"
#include "DataIndivSynapse.h"

#undef MD
#define MD 2147483648.



extern int nrn_use_selfqueue_;
//extern void nrn_pending_selfqueue(double, NrnThread*);
extern void ncs2nrn_integrate(double tstop);
 void nrn_fake_fire(int gid, double firetime, int fake_out);
extern void nrn_partrans_clear();

typedef std::map<int, SynapseInterface*> Gid2PreSyn;
class NetParEvent;

#define PreSyn SynapseInterface

class NetPar : public ParSpike
{
private:
	//static void sppk(std::vector<unsigned char> c, int gid);
	static void sppk(unsigned char* c, int gid);
	static int spupk(std::vector<unsigned char> c);
public:
	NetPar(void);
//	const DataCommonNeuron::ListSynMechType & DataCommonNeuron::presynmechlist_impl();
//	const ListSynMechType & presynmechlist_impl() const {return presyn_mech_list_;};
	static void alloc_space();
	static int spike_compress(int nspike, bool gid_compress, int xchng_meth);
	static void spike_exchange_compressed();	
	int nrn_need_npe();
	static void gid_clear();
	void spike_exchange_init();
	static double set_mindelay(double maxdelay);
	void timeout(int);
	void spike_exchange();
	void nrn2ncs_outputevent(int netcon_output_index, double firetime);
	void outputevent(unsigned char localgid, double firetime);
	void nrn_outputevent(unsigned char localgid, double firetime);
//	static Symbol* netcon_sym_;
	static Gid2PreSyn* gid2out_;
	static Gid2PreSyn* gid2in_;
	static double t_exchange_;
	static double dt1_; // 1/dt
	void mk_localgid_rep();
// for compressed gid info during spike exchange
	bool nrn_use_localgid_;

	static Gid2PreSyn** localmaps_;

#define NRNSTAT 1
	static int nsend_, nsendmax_, nrecv_, nrecv_useful_;
#if NRNSTAT
	std::vector<float> max_histogram_;
#endif 

	static int ocapacity_; // for spikeout_
	// require it to be smaller than  min_interprocessor_delay.
	static double wt_; // wait time for nrnmpi_spike_exchange
	static double wt1_; // time to find the PreSyns and send the spikes.
	static bool use_compress_;
	static int spfixout_capacity_;
	static int idxout_;

	static int active_;
	static double usable_mindelay_;
	static double min_interprocessor_delay_;	
	static double mindelay_; // the one actually used. Some of our optional algorithms
	static double last_maxstep_arg_;
	static NetParEvent* npe_; // nrn_nthread of them
	static int n_npe_; // just to compare with nrn_nthread


};


//inline static void sppk(std::vector<unsigned char> c, int gid) {
inline static void sppk(unsigned char* c, int gid) {
	for (register int i = ParSpike::localgid_size_-1; i >= 0; --i) {
		c[i] = gid & 255;
		gid >>= 8;
	}
}
inline static int spupk(std::vector<unsigned char> c) {

	int gid = c[0];
	for (register int i = 1; i < ParSpike::localgid_size_; ++i) {
		gid <<= 8;
		gid += c[i];
	}
	return gid;
}


#endif
