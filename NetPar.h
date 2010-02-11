// NetPar.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NETPAR_H
#define NETPAR_H

#ifdef CPPMPI
#include "ParSpike.2.h"
#else
#include "ParSpike.h"
#endif
//#include "BBS.h"

#include <utility>
#include <list>
#include <deque>
#include <map>
#include <fstream>
#include <string>
#include <boost/shared_ptr.hpp>

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
//extern void ncs2nrn_integrate(double tstop);

//extern void nrn_partrans_clear();

//class ParNetwork;

//class NetParEvent;

#define PreSyn SynapseInterface 

// Pre synapse class
/*
 *
 *
class PreSyn   //: public ConditionEvent {
{
public:
    PreSyn(double* src, ConfigBase* osrc, ConfigBase* ssrc = 0);
    ~PreSyn();
    void send(double sendtime); //, ParallelNetManager&);
    void deliver(double);//, ParallelNetManager&);
    double value() {
        return *thvar_ - threshold_;
    }
    double mindelay();
    //   void pr(const char*, double t, NetCvode*);
    //    void asf_err();
    //    int type() { return PreSynType; }
    //    void update();
    //    void disconnect(Observable*);
    //    void update_ptr(double*);
    //    void record_stmt(const char*);
    //    void record(IvocVect*, IvocVect* idvec = nil, int rec_id = 0);
    //   void record(double t);
    //    void init();
    
    //    std::list<SynapseTemplate*> ;
      
    double threshold_;
    double delay_;
    double* thvar_;
    ConfigBase* osrc_;
    ConfigBase* ssrc_;
    double* tvec_;
    double* idvec_;
    int use_min_delay_;
    int rec_id_;
    int output_index_;
    int gid_;
    bool flag_;
    double valthresh_;
    unsigned char localgid_; // compressed gid for spike transfer

#if BGPDMA
    union { // A PreSyn cannot be both a source spike generator
        // and a receiver of off-host spikes.
        BGP_DMASend* dma_send_;
        int srchost_;
    } bgp;
#endif

    static int presyn_send_mindelay_;
        static int presyn_send_direct_;
        static int presyn_deliver_netcon_;
        static int presyn_deliver_direct_;
        static int presyn_deliver_ncsend_;
    
};

*/

//! Pointer to PreSyn type
typedef boost::shared_ptr< PreSyn > PreSynPtr;
//! Critical typedef map of global IDs to Presynapses
typedef std::map<int, boost::shared_ptr<PreSyn> > Gid2PreSyn; //SynapseInterface
typedef std::map<int, boost::shared_ptr<PreSyn > >::iterator Gid2PreSynItr;

    // Hash table of Global IDs to PreSyn synapses
    /* Used for automated identification of neurons from IDs and PreSyn.
     *  See NEURON's original Hash table Gid2PreSyn
     */ 
  // Gid2PreSyn is very close  to NEURON's hash table for nrnmpi 
  
static Gid2PreSyn* gid2out_ = new Gid2PreSyn;
static Gid2PreSyn* gid2in_ = new Gid2PreSyn;





//! NetPar class
/*! 
 * Holds important global methods outside of the ParNetwork and BBS utilities
 * that need a link to MPI
 */
class NetPar : public ParSpike
{
private:
    
  inline static void sppk(unsigned char* c, int gid) {/*! Pack Spikes */
        for (register int i = ParSpike::localgid_size_ - 1; i >= 0; --i) {
            c[i] = gid & 255;
            gid >>= 8;
        }
    }

  inline static int spupk(unsigned char* c) {/*! Unpack Spikes */
        int gid = c[0];
        for (register int i = 1; i < ParSpike::localgid_size_; ++i) {
            gid <<= 8;
            gid += c[i];
        }
        return gid;
    }
public:
    NetPar(void);
//  const DataCommonNeuron::ListSynMechType & DataCommonNeuron::presynmechlist_impl();
//  const ListSynMechType & presynmechlist_impl() const {return presyn_mech_list_;};
    static void alloc_space();
    static int spike_compress(int nspike, bool gid_compress, int xchng_meth);
    static void spike_exchange_compressed();
    int nrn_need_npe();
    static void gid_clear();
    void spike_exchange_init();
    static double set_mindelay(double maxdelay);
    static void timeout(int);
    static void spike_exchange();
    void nrn2ncs_outputevent(int netcon_output_index, double firetime);
    void outputevent(unsigned char localgid, double firetime);
    void nrn_outputevent(unsigned char localgid, double firetime);
    void nrn_fake_fire(int gid, double firetime, int fake_out);

    /* //! map of global neuron IDs to output presynapses */
    /* static Gid2PreSyn* gid2out_; */
    /* //! map of global neuron IDs to input presynapses */
    /* static Gid2PreSyn* gid2in_; */


    static double t_exchange_;
    static double dt1_; // 1/dt
    static void mk_localgid_rep();

    //! for compressed gid info during spike exchange
    static bool nrn_use_localgid_;

    static Gid2PreSyn** localmaps_;

    static std::deque< std::pair<int, double> > tq;
#define NRNSTAT 1
    static int nsend_, nsendmax_, nrecv_, nrecv_useful_;
#if NRNSTAT
    static std::vector<double> max_histogram_;
#endif

    static int ocapacity_; /// for spikeout_ 
    static double wt_; /// wait time for nrnmpi_spike_exchange require it to be smaller than  min_interprocessor_delay.
    static double wt1_; /// time to find the PreSyns and send the spikes.
    static bool use_compress_;
    static int spfixout_capacity_;
    static int idxout_;

    static int active_;
    static double usable_mindelay_;
    static double min_interprocessor_delay_;
    static double mindelay_; // the one actually used. Some of our optional algorithms
    static double last_maxstep_arg_;
    //    static NetParEvent* npe_; // nrn_nthread of them
    static int n_npe_; // just to compare with nrn_nthread


};


#endif
