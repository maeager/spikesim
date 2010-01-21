
#include "NetPar.h"
#include "SimulationEnvironment.h"
//#include "ParSpike.h"
//#include "ParNetwork.h"
#include "BBS.h"
//#include "ParNetwork2BBS.h"

#include "AnyBuf.h"
#include <errno.h>
#include <algorithm>
#define nil 0
#undef MD
#define MD 2147483648.


// hash table where buckets are binary search maps
//implementNrnHash(Gid2PreSyn, int, SynapseInterface*)
extern ParNetwork* net_cvode_instance;

int cvode_active_ = 0;
int NetPar::ocapacity_; // for spikeout_
// require it to be smaller than  min_interprocessor_delay.
double NetPar::wt_; // wait time for nrnmpi_spike_exchange
double NetPar::wt1_; // time to find the PreSyns and send the spikes.
bool NetPar::use_compress_;
int NetPar::spfixout_capacity_;
int NetPar::idxout_;
Gid2PreSyn** NetPar::localmaps_ = new Gid2PreSyn*[1];
int NetPar::active_;
double NetPar::usable_mindelay_;
double NetPar::min_interprocessor_delay_;
double NetPar::mindelay_; // the one actually used. Some of our optional algorithms
double NetPar::last_maxstep_arg_;
NetParEvent* NetPar::npe_; // nrn_nthread of them
int NetPar::n_npe_; // just to compare with nrn_nthread
Gid2PreSyn* NetPar::gid2out_ = new Gid2PreSyn;
Gid2PreSyn* NetPar::gid2in_ = new Gid2PreSyn;
double NetPar::t_exchange_;
double NetPar::dt1_; // 1/dt
bool NetPar::nrn_use_localgid_;
int NetPar::nsend_ = 0;
int NetPar::nsendmax_ = 0;
int NetPar::nrecv_ = 0;
int NetPar::nrecv_useful_ = 0;
int nrn_use_selfqueue_;
#if NRNSTAT
std::vector<double> NetPar::max_histogram_;
#endif


PreSyn::PreSyn(double* src, ConfigBase* osrc, ConfigBase* ssrc)
{

//  hi_index_ = -1;
//  hi_th_ = nil;
    flag_ = false;
    valthresh_ = 0;
    thvar_ = src;
    osrc_ = osrc;
    ssrc_ = ssrc;
    threshold_ = 10.;
    use_min_delay_ = 0;
    tvec_ = nil;
    idvec_ = nil;
//  stmt_ = nil;
    gid_ = -1;

    output_index_ = -1;

#if BGPDMA
    bgp.dma_send_ = 0;
#endif

}

PreSyn::~PreSyn()
{

}

NetPar::NetPar(void)
{

}

class NetParEvent
{
public:
    NetParEvent();
    ~NetParEvent();
//send(double tt, NetCvode* nc, NrnThread* nt)
//deliver(double tt, NetCvode* nc, NrnThread* nt)
//pgvts_deliver(double tt, NetCvode* nc)
    double wx_, ws_;
    int ithread_;
};

NetParEvent::NetParEvent()
{
    wx_ = ws_ = 0.;
    ithread_ = -1;
}
NetParEvent::~NetParEvent()
{
}
/*
void NetParEvent::send(double tt, NetCvode* nc, NrnThread* nt){
    nc->event(tt + usable_mindelay_, this, nt);
}

void NetParEvent::deliver(double tt, NetCvode* nc, NrnThread* nt){
    if (nrn_use_selfqueue_) { //first handle pending flag=1 self events
        nrn_pending_selfqueue(tt, nt);
    }
    // has to be the last event at this time in order to avoid a race
    // condition with HocEvent that may call things such as pc.barrier
    // actually allthread HocEvent (cvode.event(tev) and cvode.event(tev,stmt)
    // will be executed last after a thread join when nrn_allthread_handle
    // is called.
    net_cvode_instance->deliver_events(tt, nt);
    nt->_stop_stepping = 1;
    nt->_t = tt;

    if (numprocs > 0 && nt->id == 0) {
    nrn_spike_exchange();
    wx_ += NetPar::wt_;
    ws_ += NetPar::wt1_;
   }

    send(tt, nc, nt);
}
void NetParEvent::pgvts_deliver(double tt, NetCvode* nc){
    assert(0);
    deliver(tt, nc, 0);
}

void NetParEvent::pr(const char* m, double tt, NetCvode* nc){
    std::cout <<m<< " NetParEvent " << ithread_ <<" t= "<<setwidth(15) << tt << " tt-t= " << tt - nrn_threads[ithread_]._t << std::endl;
}
*/





void NetPar::nrn_outputevent(unsigned char localgid, double firetime)
{
    if (!active_) {
        return;
    }
    nout_++;
    int i = idxout_;
    idxout_ += 2;
    if (idxout_ >= spfixout_capacity_) {
        spfixout_capacity_ *= 2;
        spfixout_.resize(spfixout_capacity_);
    }
    spfixout_[i++] = (unsigned char)((firetime - t_exchange_) * dt1_ + .5);
    spfixout_[i] = localgid;
#ifdef DEBUG
    std::cout << ParSpike::my_rank << " idx="<< i<<" lgid="<< (int)localgid<<" firetime="<<firetime<<" t_exchange_="<<t_exchange_<<" [0]="<<(int)spfixout_[i-1]<<" [1]="<< (int)spfixout_[i] << std::endl;
#endif
}


void NetPar::nrn2ncs_outputevent(int gid, double firetime)
{
    if (!active_) {
        return;
    }
    SpikePacket_* temp;
    int i,idx;
    if (use_compress_) {
        nout_++;
        i = idxout_;
        idxout_ += 1 + localgid_size_;
        if (idxout_ >= spfixout_capacity_) {
            spfixout_capacity_ *= 2;
            spfixout_.resize(spfixout_capacity_);
        }
#ifdef DEBUG
std::cout << ParSpike::my_rank << " nrnncs_outputevent "<<gid<<" " << firetime <<" "<< t_exchange_ <<" "<<(int)((unsigned char)((firetime - t_exchange_)*dt1_ + .5))<< std::endl;
#endif
	spfixout_[i++] = (unsigned char)((firetime - t_exchange_) * dt1_ + .5);
std::cout<<  my_rank <<" idx= " <<  i<< "  firetime=" <<  firetime <<" t_exchange_="<<  t_exchange_ << " spfixout="<< (int)spfixout_[i-1] << std::endl;
        sppk(&spfixout_[i], gid); 
//std::cout<<  my_rank <<" idx= " <<  i<< "  gid=" <<  gid <<" spupk="<<  spupk(&spfixin_ovfl_[idx]) << std::endl;
    } else {
#if nrn_spikebuf_size == 0
	i = nout_++;
        if (i >= ocapacity_) {
            ocapacity_ *= 2;
            spikeout_.resize(ocapacity_);
        }
std::cout<<  my_rank <<" cell  " <<  gid<< "  in slot " <<  i <<" fired at "<<  firetime << std::endl;
        spikeout_[i].gid = gid;
        spikeout_[i].spiketime = firetime;
#else
	i = nout_++;
        if (i >= nrn_spikebuf_size) {
            i -= nrn_spikebuf_size;
            if (i >= ocapacity_) {
                ocapacity_ *= 2;
                spikeout_.resize(ocapacity_);
            }
            spikeout_[i].gid = gid;
            spikeout_[i].spiketime = firetime;
        } else {
            spbufout_->gid[i] = gid;
            spbufout_->spiketime[i] = firetime;
        }
#endif
    }
std::cout<<  my_rank <<" cell  " <<  gid<< "  in slot " <<  i <<" fired at "<<  firetime << std::endl;
}


int NetPar::nrn_need_npe()
{

    int b = 0;
    if (active_) {
        b = 1;
    }
    if (nrn_use_selfqueue_) {
        b = 1;
    }
//  if (nrn_nthread > 1) { b = 1; }
    if (b) {
        if (last_maxstep_arg_ == 0) {
            last_maxstep_arg_ =   100.;
        }
        set_mindelay(last_maxstep_arg_);
    } else {
        if (npe_) {
            delete [] npe_;
            npe_ = nil;
            n_npe_ = 0;
        }
    }
    return b;
}
/*
void NetPar::calc_actual_mindelay() {
    //reasons why mindelay_ can be smaller than min_interprocessor_delay
    // are use_bgpdma when BGP_INTERVAL == 2
#if BGPDMA && (BGP_INTERVAL == 2)
    if (use_bgpdma_) {
        mindelay_ = min_interprocessor_delay_ / 2.;
    }else{
        mindelay_ = min_interprocessor_delay_;
    }
#endif
}
*/
void NetPar::spike_exchange_init()
{
#ifdef DEBUG
std::cout <<"NetPar::spike_exchange_init" <<std::endl;
#endif
    if (!nrn_need_npe()) {
        return;
    }
//  if (!active_ && !nrn_use_selfqueue_) { return; }
    alloc_space();
//std::cout<< " use= " <<  use<< "  active=" <<  active_<< std::endl;
    //calc_actual_mindelay();
    usable_mindelay_ = mindelay_;
    if (cvode_active_ == 0) {//&& nrn_nthread > 1) {
        usable_mindelay_ -= SimEnv::timestep(); //dt;
    }
    if ((usable_mindelay_ < 1e-9) || (cvode_active_ == 0 && usable_mindelay_ < SimEnv::timestep())) {
        if (my_rank == 0) {
	    std::cerr << "usable mindelay is 0 (or less than dt for fixed step method \n";
        } else {
            return;
        }
    }

//  if (n_npe_ != nrn_nthread) {
//      if (npe_) { delete [] npe_; }
//      npe_ = new NetParEvent[nrn_nthread];
//      n_npe_ = nrn_nthread;
//  }
//  for (int i = 0; i < nrn_nthread; ++i) {
//      npe_[i].ithread_ = i;
//      npe_[i].wx_ = 0.;
//      npe_[i].ws_ = 0.;
//      npe_[i].send(t, net_cvode_instance, nrn_threads + i);
//  }

    if (use_compress_) {
        idxout_ = 2;
        t_exchange_ =  SimEnv::i_time();//t;
        dt1_ = 1. / SimEnv::timestep();
        usable_mindelay_ = floor(mindelay_ * dt1_ + 1e-9) * SimEnv::timestep();
        assert(usable_mindelay_ >= SimEnv::timestep() && (usable_mindelay_ * dt1_) < 255);
    } else {
#if nrn_spikebuf_size > 0
        if (spbufout_) {
            spbufout_->nspike = 0;
        }
#endif
    }
    nout_ = 0;
    nsend_ = nsendmax_ = nrecv_ = nrecv_useful_ = 0;

    if (my_rank == 0){
    std::cout <<"usable_mindelay_ = "<< usable_mindelay_<<std::endl;
    }
}


void NetPar::spike_exchange()
{
    if (!active_) {
        return;
    }
    if (use_compress_) {
        spike_exchange_compressed(); return;
    }
    double wt;
    int i, n;
#if NRNSTAT
    nsend_ += nout_;
    if (nsendmax_ < nout_) {
        nsendmax_ = nout_;
    }
#endif
#if nrn_spikebuf_size > 0
    spbufout_->nspike = nout_;
#endif
    wt = wtime();
    n = ParSpike::spike_exchange();
    NetPar::wt_ = wtime() - wt;
    wt = wtime();
    errno = 0;
#ifdef DEBUG
if (n > 0) {
std::cout<<  my_rank <<" spike_exchange sent  " <<  nout_<< "  received "<<  n<< std::endl;
}
#endif
    nout_ = 0;
    if (n == 0) {
#if NRNSTAT
        //if (max_histogram_)
        { max_histogram_[0] += 1.; }
#endif
        return;
    }
#if NRNSTAT
    nrecv_ += n;
//  if (max_histogram_) {
    int mx = 0;
    if (n > 0) {
        for (i = numprocs - 1 ; i >= 0; --i) {
#if nrn_spikebuf_size == 0
            if (mx < nin_[i]) {
                mx = nin_[i];
            }
#else
            if (mx < spbufin_[i].nspike) {
                mx = spbufin_[i].nspike;
            }
#endif
        }
    }
    int ms = max_histogram_.size() - 1;
    mx = (mx < ms) ? mx : ms;
    max_histogram_[mx] += 1.;
//  }
#endif // NRNSTAT
#if nrn_spikebuf_size > 0
    for (i = 0; i < numprocs; ++i) {
        int j;
        int nn = spbufin_[i].nspike;
        if (nn > nrn_spikebuf_size) {
            nn = nrn_spikebuf_size;
        }
        for (j = 0; j < nn; ++j) {
            PreSynPtr ps;
            if (ps = gid2in_->find(spbufin_[i].gid[j])->second)) {
//TODO              ps->send(spbufin_[i].spiketime[j], net_cvode_instance);//, nrn_threads);
#if NRNSTAT
                ++nrecv_useful_;
#endif
            }
        }
    }
    n = ovfl_;
#endif // nrn_spikebuf_size > 0
    for (i = 0; i < n; ++i) {
        PreSynPtr ps;
        if (ps = gid2in_->find(spikein_[i].gid)->second) {
//TODO          ps->send(spikein_[i].spiketime, net_cvode_instance));//, nrn_threads);
#if NRNSTAT
            ++nrecv_useful_;
#endif
        }
    }
    NetPar::wt1_ = wtime() - wt;
}

void NetPar::spike_exchange_compressed()
{
    if (!active_) {
        return;
    }
    assert(!cvode_active_);
    double wt;
    int i, n, idx;
#if NRNSTAT
    nsend_ += nout_;
    if (nsendmax_ < nout_) {
        nsendmax_ = nout_;
    }
#endif
    assert(nout_ < 0x10000);
    spfixout_[1] = (unsigned char)(nout_ & 0xff);
    spfixout_[0] = (unsigned char)(nout_ >> 8);

    wt = wtime();
    n = ParSpike::spike_exchange_compressed();
    wt_ = wtime() - wt;
    wt = wtime();
    errno = 0;
//if (n > 0) {
std::cout<<  my_rank <<" nrn_spike_exchange sent  " <<  nout_<< "  received "<<  n<< std::endl;
//}
    nout_ = 0;
    idxout_ = 2;
    if (n == 0) {
#if NRNSTAT
        //if (max_histogram_)
        { max_histogram_[0] += 1.; }
#endif
        t_exchange_ = SimEnv::i_time();//t; // nrn_threads->_t;
        return;
    }
#if NRNSTAT
    nrecv_ += n;
    //if (max_histogram_) {
    int mx = 0;
    if (n > 0) {
        for (i = numprocs - 1 ; i >= 0; --i) {
            if (mx < nin_[i]) {
                mx = nin_[i];
            }
        }
    }
    int ms = (max_histogram_.capacity()) - 1;
    mx = (mx < ms) ? mx : ms;
    max_histogram_[mx] += 1.;
    //}
#endif // NRNSTAT
    if (nrn_use_localgid_) {
        int idxov = 0;
        for (i = 0; i < numprocs; ++i) {
            int j, nnn;
            int nn = nin_[i];
            if (nn) {
                if (i == my_rank) { // skip but may need to increment idxov.
                    if (nn > ag_send_nspike_) {
                        idxov += (nn - ag_send_nspike_) * (1 + localgid_size_);
                    }
                    continue;
                }
                Gid2PreSyn* gps = localmaps_[i];
                if (nn > ag_send_nspike_) {
                    nnn = ag_send_nspike_;
                } else {
                    nnn = nn;
                }
                idx = 2 + i * ag_send_size_;
                for (j = 0; j < nnn; ++j) {
                    // order is (firetime,gid) pairs.
                    double firetime = spfixin_[idx++] * SimEnv::timestep() + t_exchange_;
                    int lgid = (int)spfixin_[idx];
                    idx += localgid_size_;
                    PreSynPtr ps = gps->find(lgid)->second;
                    if (ps) {
//TODO              ps->send(firetime + 1e-10, net_cvode_instance);//, nrn_threads);
#if NRNSTAT
                        ++nrecv_useful_;
#endif
                    }
                }
                for (; j < nn; ++j) {
                    double firetime = spfixin_ovfl_[idxov++] * SimEnv::timestep() + t_exchange_;
                    int lgid = (int)spfixin_ovfl_[idxov];
                    idxov += localgid_size_;
                    PreSynPtr ps;
                    if (ps  = gps->find(lgid)->second) {
//TODO              ps->send(firetime+1e-10, net_cvode_instance);//, nrn_threads);
#if NRNSTAT
                        ++nrecv_useful_;
#endif
                    }
                }
            }
        }
    } else {
        for (i = 0; i < numprocs; ++i) {
            int j;
            int nn = nin_[i];
            if (nn > ag_send_nspike_) {
                nn = ag_send_nspike_;
            }
            idx = 2 + i * ag_send_size_;
            for (j = 0; j < nn; ++j) {
                // order is (firetime,gid) pairs.
                double firetime = spfixin_[idx++] * SimEnv::timestep() + t_exchange_;
                int gid = spupk(&spfixin_ovfl_[idx]);//(spfixin_ + idx);
                idx += localgid_size_;
                PreSynPtr ps;
                if (ps  = gid2in_->find(gid)->second) {
//TODO              ps->send(firetime+1e-10, net_cvode_instance);//, nrn_threads);
#if NRNSTAT
                    ++nrecv_useful_;
#endif
                }
            }
        }
        n = ovfl_;
        idx = 0;
        for (i = 0; i < n; ++i) {
            double firetime = spfixin_ovfl_[idx++] * SimEnv::timestep() + t_exchange_;
            int gid = spupk(&spfixin_ovfl_[idx]);//(spfixin_ovfl_ + idx);
            idx += localgid_size_;
            PreSynPtr ps;
            if (ps = gid2in_->find(gid)->second) {
//TODO          ps->send(firetime+1e-10, net_cvode_instance);//, nrn_threads);
#if NRNSTAT
                ++nrecv_useful_;
#endif
            }
        }
    }
    t_exchange_ = SimEnv::i_time();//nrn_threads->_t;
    NetPar::wt1_ = wtime() - wt;
}


void NetPar::mk_localgid_rep()
{
    int i, j, k;
    PreSynPtr ps;

    // how many gids are there on this machine
    // and can they be compressed into one byte
    int ngid = 0;
//TODO
//NrnHashIterate(Gid2PreSyn, gid2out_, PreSyn*, ps) {
    for (Gid2PreSyn::const_iterator i = NetPar::gid2out_->begin();
            i != NetPar::gid2out_->end();
            ++i) {
        ps = i->second;
        if (ps->output_index_ >= 0) {
            ++ngid;
        }
    }
    int ngidmax = int_allmax(ngid);
    if (ngidmax >= 256) {
        //do not compress
        return;
    }
    localgid_size_ = sizeof(unsigned char);
    nrn_use_localgid_ = true;

    // allocate Allgather receive buffer (send is the my_rank one)
    int* rbuf = new int[numprocs*(ngidmax + 1)];
    int* sbuf = rbuf + my_rank * (ngidmax + 1);

    sbuf[0] = ngid;
    ++sbuf;
    ngid = 0;
    // define the local gid and fill with the gids on this machine
//TODO
//NrnHashIterate(Gid2PreSyn, gid2out_, PreSyn*, ps) {
    for (Gid2PreSyn::const_iterator i = NetPar::gid2out_->begin();
            i != NetPar::gid2out_->end();
            ++i) {
        ps = i->second;
        if (ps->output_index_ >= 0) {
            ps->localgid_ = (unsigned char)ngid;
            sbuf[ngid] = ps->output_index_;
            ++ngid;
        }
    }
    --sbuf;

    // exchange everything
    int_allgather(sbuf, rbuf, ngidmax + 1);
    errno = 0;

    // create the maps
    // there is a lot of potential for efficiency here. i.e. use of
    // perfect hash functions, or even simple Vectors.
    localmaps_ = new Gid2PreSyn*[numprocs];
    localmaps_[my_rank] = 0;
    for (i = 0; i < numprocs; ++i) if (i != my_rank) {
            // how many do we need?
            sbuf = rbuf + i * (ngidmax + 1);
            ngid = *(sbuf++); // at most
            // of which we actually use...
            for (k = 0, j = 0; k < ngid; ++k) {
                if (gid2in_ && ps /*TODO =gid2in_->find(int(sbuf[k]))->second*/) {
                    ++j;
                }
            }
            // oh well. there is probably a rational way to choose but...
            localmaps_[i] = new Gid2PreSyn;//((j > 19) ? 19 : j+1);
        }

    // fill in the maps
    for (i = 0; i < numprocs; ++i) if (i != my_rank) {
            sbuf = rbuf + i * (ngidmax + 1);
            ngid = *(sbuf++);
            for (k = 0; k < ngid; ++k) {
                if (gid2in_ && ps /*TODO =gid2in_->find(int(sbuf[k]))->second*/) {
//TODO              (*localmaps_[i])[k] = ps;
                }
            }
        }

    // cleanup
    delete [] rbuf;
}



// may stimulate a gid for a cell not owned by this cpu. This allows
// us to run single cells or subnets and stimulate exactly according to
// their input in a full parallel net simulation.
// For some purposes, it may be useful to simulate a spike from a
// cell that does exist and would normally send its own spike, eg.
// recurrent stimulation. This can be useful in debugging where the
// spike raster comes from another implementation and one wants to
// get complete control of all input spikes without the confounding
// effects of output spikes from the simulated cells. In this case
// set the third arg to 1 and set the output cell thresholds very
// high so that they do not themselves generate spikes.
void NetPar::nrn_fake_fire(int gid, double spiketime, int fake_out)
{
    assert(gid2in_);
    PreSynPtr ps = gid2in_->find(gid)->second;
    if (ps) {
        assert(ps);
#ifdef DEBUG
std::cout << "nrn_fake_fire "<< gid <<" "<< spiketime<<std::endl;
#endif
	//TODO      ps->send(spiketime, net_cvode_instance);//, nrn_threads);
#if NRNSTAT
        ++nrecv_useful_;
#endif
    } else if (fake_out && ps) {
        assert(ps);
#ifdef DEBUG
std::cout << "nrn_fake_fire fake_out " << gid <<" "<<  spiketime<< std::endl;
#endif
//TODO      ps->send(spiketime, net_cvode_instance);//, nrn_threads);
#if NRNSTAT
        ++nrecv_useful_;
#endif
    }

}

void NetPar::alloc_space()
{
    if (!gid2out_) {
//      netcon_sym_ = hoc_lookup("NetCon");
//      if (!gid2out_) gid2out_= boost::shared_ptr<Gid2PreSyn>();//(211);
//      if (!gid2in_) gid2in_= boost::shared_ptr<Gid2PreSyn>();//(2311);

        ocapacity_  = 100;
        spikeout_.clear(); spikeout_.resize(ocapacity_);
        //if (spikeout_) delete [] spikeout_;
        //spikeout_ = new SpikePacket_[ocapacity_];
        icapacity_  = 100;
        spikein_.clear(); spikein_.resize(icapacity_);
        //if (spikein_)
        //  delete [] spikein_;
        //spikein_ = new  SpikePacket_[icapacity_];
        if (nin_) delete nin_;
        nin_ = new int(numprocs);
#if nrn_spikebuf_size > 0
        spbufout_.resize(1);
        spbufin_.resize(numprocs);
//if (spbufout_) delete spbufout_;
//      spbufout_ = new SpikeBuffer_;
//if (spbufin_) delete spbufin_;
//      spbufin_ = new SpikeBuffer_(numprocs);
#endif

    }
}

void BBS::set_gid2node(int gid, int nid)
{
    NetPar::alloc_space();

    if (nid == my_rank) {
        std::cout<< " gid  " <<  gid<< "  defined on " <<  my_rank<< std::endl;
        //PreSyn* ps;
        if (NetPar::gid2in_->find(gid)->first)
            NetPar::gid2in_->erase(gid);//assert(!(ps = NetPar::gid2in_->find(gid)));
        (*NetPar::gid2out_)[gid] = PreSynPtr(); //temporary - TODO change this so that it points to the presynapse
//TODO      NetPar::gid2out_->insert(pair<const int, PreSynPtr>(gid, nil));
    }
}

void NetPar::gid_clear()
{
//TODO  nrn_partrans_clear();
#if PARANEURON
    nrnmpi_split_clear();
#endif

    if (!gid2out_) {
        return;
    }
    PreSynPtr ps, psi;
// NrnHashIterate(Gid2PreSyn, gid2out_, PreSyn*, ps) {
    for (Gid2PreSyn::const_iterator i = NetPar::gid2out_->begin();
            i != NetPar::gid2out_->end();
            ++i) {
        ps = i->second;
        if (ps && !(psi = NetPar::gid2in_->find(ps->gid_)->second)) {
            ps->gid_ = -1;
            ps->output_index_ = -1;
            //if (ps->dil_.count() == 0) {
            //  delete ps;
            //}
        }
    }

    for (Gid2PreSyn::const_iterator i = gid2in_->begin();
            i != gid2in_->end();
            ++i) {
        ps = i->second;
        ps->gid_ = -1;
        ps->output_index_ = -1;
        //  if (ps->dil_.count() == 0) { //No synaptic connections
        //  delete ps;
        //  }
    }

//  int i;
//  for (i = gid2out_->size_ - 1; i >= 0; --i) {
//      gid2out_->at(i).clear();
//  }
    gid2out_->clear();
//  for (i = gid2in_->size_ - 1; i >= 0; --i) {
//      gid2in_->at(i).clear();
//  }
    gid2in_->clear();
}

int BBS::gid_exists(int gid)
{
    PreSynPtr ps;
    NetPar::alloc_space();
    if (ps = NetPar::gid2out_->find(gid)->second) {
std::cout<<  my_rank <<" gid  " <<  gid<< "  exists"<< std::endl;
        if (ps) {
            return (ps->output_index_ >= 0 ? 3 : 2);
        } else {
            return 1;
        }
    }
    return 0;
}

double BBS::threshold(int gid, double threshold)
{
    PreSynPtr ps = NetPar::gid2out_->find(gid)->second;

    if (threshold == -1.0) {
        ps->threshold_ = threshold;
    }
    return ps->threshold_;

}



void BBS::cell()
{
    /*TODO  int gid = int(chkarg(1, 0., MD));
        PreSyn* ps;
        assert(gid2out_->find(gid, ps));
        Object* ob = *hoc_objgetarg(2);
        if (!ob || ob->ctemplate != netcon_sym_->u.ctemplate) {
            check_obj_type(ob, "NetCon");
        }
        NetCon* nc = (NetCon*)ob->u.this_pointer;
        ps = nc->src_;
    std::cout<<  my_rank <<" cell  " <<  gid<<" "<<  hoc_object_name(ps->ssrc_ ? ps->ssrc_->prop->dparam[6].obj : ps->osrc_) << std::endl;
        (*gid2out_)[gid] = ps;
        ps->gid_ = gid;
        if (ifarg(3) && !chkarg(3, 0., 1.)) {
            ps->output_index_ = -2; //prevents destruction of PreSyn
        }else{
            ps->output_index_ = gid;
        }
    */
}

void BBS::cell(int gid) //, NetCon* nc) {
{
    PreSynPtr   ps = NetPar::gid2out_->find(gid)->second;
    (*NetPar::gid2out_)[gid] = ps;
    ps->gid_ = gid;
    ps->output_index_ = gid;
}


void BBS::outputcell(int gid)
{
    PreSynPtr ps = NetPar::gid2out_->find(gid)->second;
//TODO  assert(ps = NetPar::gid2out_->find(gid));
    assert(ps);
    ps->output_index_ = gid;
    ps->gid_ = gid;
}
/*
void BBS::spike_record(int gid, std::vector<double>* spikevec, std::vector<double>* gidvec) {
    PreSyn* ps;
    assert(gid2out_->find(gid, ps));
    assert(ps);
    ps->record(spikevec, gidvec, gid);
}
*/

ConfigBase* BBS::gid2obj(int gid)
{
    ConfigBase* cell = 0;
std::cout<<  my_rank <<" gid2obj gid= " <<  gid<< std::endl;
    PreSynPtr ps;
    assert(ps = NetPar::gid2out_->find(gid)->second);
std::cout<< "  found " << std::endl;
//  assert(ps);
//TODO  cell = ps->ssrc_ ? ps->ssrc_->prop->dparam[6].obj : ps->osrc_;
std::cout<< "  return  " <<  (cell)<< std::endl;
    return cell;
}

ConfigBase* BBS::gid2cell(int gid)
{
    ConfigBase* cell = 0;
std::cout<<  my_rank <<" gid2obj gid= " <<  gid<< std::endl;
    PreSynPtr ps;
    assert(ps = NetPar::gid2out_->find(gid)->second);
std::cout<< "  found " << std::endl;
    assert(ps);
    /*TODO  if (ps->ssrc_) {
            cell = ps->ssrc_->prop->dparam[6].obj;
        }else{
            cell = ps->osrc_;
            // but if it is a POINT_PROCESS in a section
            // that is inside an object ... (probably has a WATCH statement)
            Section* sec = ob2pntproc(cell)->sec;
            if (sec && sec->prop->dparam[6].obj) {
                cell = sec->prop->dparam[6].obj;
            }
        }
    */
//std::cout<< "  return  " <<  hoc_object_name(cell)<< std::endl;
    return cell;
}

ConfigBase* BBS::gid_connect(int gid, ConfigBase* target)
{
    /*  if (!is_point_process(target)) {
            hoc_execerror("arg 2 must be a point process", 0);
        }
    */
    NetPar::alloc_space();
    PreSynPtr ps;
    if (ps  = NetPar::gid2out_->find(gid)->second) {
        // the gid is owned by this machine so connect directly
        assert(ps);
    } else if ((ps = NetPar::gid2in_->find(gid)->second)) {
        // the gid stub already exists
std::cout<<  my_rank <<" connect  " <<  target->gid<< "  from already existing "<<  gid<< std::endl;
    } else {
std::cout<<  my_rank <<" connect  " <<  target->gid  << "  from new PreSyn for "<<  gid<< std::endl;
        PreSyn* ps_ = new PreSyn(nil, nil, nil);//,target);
        ps = PreSynPtr(ps_); //(nil, nil, nil);
//TODO?     net_cvode_instance->psl_append(ps);
        (*NetPar::gid2in_)[gid] = ps;
        ps->gid_ = gid;
    }
    ConfigBase** po;
    /*TODO  NetCon* nc;

        if (ifarg(3)) {
            po = hoc_objgetarg(3);
            if (!*po || (*po)->ctemplate != netcon_sym_->u.ctemplate) {
                check_obj_type(*po, "NetCon");
            }
            nc = (NetCon*)((*po)->u.this_pointer);
            if (nc->target_ != ob2pntproc(target)) {
                std::cout <<"target is different from 3rd arg NetCon target" << std::endl;
            }
            nc->replace_src(ps);
        }else{
            nc = new NetCon(ps, target);
            po = hoc_temp_objvar(netcon_sym_, nc);
            nc->obj_ = *po;
        }
    */  return *po;

}

void BBS::netpar_solve(double tstop)
{

    double mt, md;
//  tstopunset;
    if (cvode_active_) {
        mt = 1e-9 ; md = NetPar::mindelay_;
    } else {
        mt = SimEnv::timestep() ; md = NetPar::mindelay_ - 1e-10;
    }
    if (md < mt) {
        if (my_rank == 0) {
            std::cout << "mindelay is 0 (or less than dt for fixed step method)" << std::endl;
        } else {
            return;
        }
    }
    double wt;

    NetPar::timeout(20);
    wt = wtime();
    if (cvode_active_) {
//TODO      ncs2nrn_integrate(tstop);
    } else {
//TODO      ncs2nrn_integrate(tstop+1e-11);
    }
    impl_->integ_time_ += wtime() - wt;
    impl_->integ_time_ -= (NetPar::npe_ ? (NetPar::npe_[0].wx_ + NetPar::npe_[0].ws_) : 0.);

    NetPar::spike_exchange();

    NetPar::timeout(0);
    impl_->wait_time_ += NetPar::wt_;
    impl_->send_time_ += NetPar::wt1_;
    if (NetPar::npe_) {
        impl_->wait_time_ += NetPar::npe_[0].wx_;
        impl_->send_time_ += NetPar::npe_[0].ws_;
        NetPar::npe_[0].wx_ = NetPar::npe_[0].ws_ = 0.;
    };
std::cout<< my_rank <<" netpar_solve exit t= " <<  SimEnv::sim_time()<< "  tstop=" << SimEnv::i_duration()*SimEnv::timestep() <<" mindelay_="<< NetPar::mindelay_ << std::endl;

//  tstopunset;
}

double PreSyn::mindelay()
{
    double md = 1e9;
    /*TODO  int i;
        for (i=dil_.count()-1; i >= 0; --i) {
            NetCon* d = dil_.item(i);
            if (md > d->delay_) {
                md = d->delay_;
            }
        }
    */  return md;

}

double NetPar::set_mindelay(double maxdelay)
{
    double mindelay = maxdelay;
    last_maxstep_arg_ = maxdelay;
// TODO    if (nrn_use_selfqueue_ || net_cvode_instance->localstep()  ) {//|| nrn_nthread > 1
//  hoc_Item* q;
//  if (net_cvode_instance->psl_) ITERATE(q, net_cvode_instance->psl_) {
//      PreSyn* ps = (PreSyn*)VOIDITM(q);
//      double md = ps->mindelay();
//      if (mindelay > md) {
//          mindelay = md;
//      }
//  }
//     }

//    else{
//  NrnHashIterate(Gid2PreSyn, gid2in_, PreSyn*, ps) {
    PreSynPtr ps;
    for (Gid2PreSyn::const_iterator i = NetPar::gid2in_->begin();
            i != NetPar::gid2in_->end();
            ++i) {
        ps = i->second;
        double md = ps->mindelay();
        if (mindelay > md) {
            mindelay = md;
        }
    }
//    }
    if (mpi_use) {
        active_ = 1;
    }
    if (use_compress_) {
        if (mindelay_ / SimEnv::timestep() > 255) {
            mindelay_ = 255 * SimEnv::timestep();
        }
    }

std::cout<<  my_rank <<" netpar_mindelay local  " <<  mindelay<< "  now calling nrnmpi_mindelay"<< std::endl;
//  double st = time();
    mindelay_ = ParSpike::mindelay(mindelay_);
    min_interprocessor_delay_ = mindelay_;
//  add_wait_time(st);
std::cout<<  my_rank <<" local min= " <<  mindelay<< "   global min="<<  mindelay_<< std::endl;
    if (mindelay_ < 1e-9 && nrn_use_selfqueue_) {
        nrn_use_selfqueue_ = 0;
        double od = mindelay_;
        mindelay_ = set_mindelay(maxdelay);
        if (my_rank == 0) {
            std::cout << "Notice: The global minimum NetCon delay is " << od << ", so turned off the cvode.queue_mode\n" << std::endl;
            std::cout << "   use_self_queue option. The interprocessor minimum NetCon delay is " <<  mindelay_ << std::endl;
        }
    }
#if BGPDMA
    if (use_bgpdma_) {
        bgp_dma_setup();
    }
#endif
    errno = 0;
    return mindelay;

}

double BBS::netpar_mindelay(double maxdelay)
{
    return NetPar::set_mindelay(maxdelay);
}

void BBS::netpar_spanning_statistics(int* nsend, int* nsendmax, int* nrecv, int* nrecv_useful)
{
    *nsend = NetPar::nsend_;
    *nsendmax = NetPar::nsendmax_;
    *nrecv = NetPar::nrecv_;
    *nrecv_useful = NetPar::nrecv_useful_;
}

// unfortunately, ivocvect.h conflicts with STL
std::vector<double> BBS::netpar_max_histogram(std::vector<double> mh)
{
    /*TODO
        std::vector<double> h = NetPar::max_histogram_;
        if (NetPar::max_histogram_) {
            NetPar::max_histogram_ = nil;
        }
        if (mh) {
            NetPar::max_histogram_ = *mh;
        }
        return h;
    */

}

int NetPar::spike_compress(int nspike, bool gid_compress, int xchng_meth)
{

    if (numprocs < 2) {
        return 0;
    }
#if BGPDMA
//  use_bgpdma_ = (xchng_meth == 1) ? 1 : 0;
//  if (my_rank == 0) {printf("use_bgpdma_ = " << use_bgpdma_;}
#endif
    if (nspike >= 0) {
        ag_send_nspike_ = 0;
        spfixout_.resize(0); //if (spfixout_) { spfixout_ = 0; }
        spfixin_.resize(0);//if (spfixin_) {  spfixin_ = 0; }
        spfixin_ovfl_.resize(0); //if (spfixin_ovfl_) { spfixin_ovfl_.resize(0); spfixin_ovfl_ = 0; }
        if (localmaps_) {
            for (int i = 0; i < numprocs; ++i) if (i != my_rank) {
                    if (localmaps_[i]) {
                        delete localmaps_[i];
                    }
                }
            delete [] localmaps_;
            localmaps_ = 0;
        }
    }
    if (nspike == 0) { // turn off
        use_compress_ = false;
        nrn_use_localgid_ = false;
    } else if (nspike > 0) { // turn on
        if (cvode_active_) {
            if (my_rank == 0) {
                std::cout << "ParallelContext.spike_compress cannot be used with cvode active" << std::endl;
            }
            use_compress_ = false;
            nrn_use_localgid_ = false;
            return 0;
        }
        use_compress_ = true;
        ag_send_nspike_ = nspike;
        nrn_use_localgid_ = false;
        if (gid_compress) {
            // we can only do this after everything is set up
            mk_localgid_rep();
            if (!nrn_use_localgid_ && my_rank == 0) {
                std::cout << "Notice: gid compression did not succeed. Probably more than 255 cells on one cpu.\n" << std::endl;
            }
        }
        if (!nrn_use_localgid_) {
            localgid_size_ = sizeof(unsigned int);
        }
        ag_send_size_ = 2 + ag_send_nspike_ * (1 + localgid_size_);
        spfixout_capacity_ = ag_send_size_ + 50 * (1 + localgid_size_);
        spfixout_.resize(spfixout_capacity_);
        spfixin_.resize(numprocs);
        ovfl_capacity_ = 100;
        spfixin_ovfl_.resize(ovfl_capacity_);
    }
    return ag_send_nspike_;

}

#include <signal.h>
#include <sys/time.h>


static double told;
static struct itimerval value;
#if !defined(BLUEGENE)
static struct sigaction act, oact;
#endif

static void timed_out(int sig)
{
#if 0
    printf("timed_out told=%g t=%g\n", told, t);
#endif
    if (SimEnv::sim_time() == told) { /* nothing has been accomplished since last signal*/
        std::cout << "nrn_timeout t= " << SimEnv::sim_time() << std::endl;
        ParSpike::mpiabort(0);
    }
    told = SimEnv::sim_time();
}

void NetPar::timeout(int seconds)
{
    if (ParSpike::my_rank != 0) {
        return;
    } //only the Master continues
#if 0
    printf("nrn_timeout %d\n", seconds);
#endif
#if BLUEGENE
    if (seconds) {
        //t = told;
        signal(SIGALRM, timed_out);
    } else {
        signal(SIGALRM, SIG_DFL);
    }
#else
    if (seconds) {
        //t = told;
        act.sa_handler = timed_out;
        act.sa_flags = SA_RESTART;
        if (sigaction(SIGALRM, &act, &oact)) {
            std::cout << "sigaction failed" << std::endl;
            ParSpike::mpiabort(0);
        }
    } else {
        sigaction(SIGALRM, &oact, (struct sigaction*)0);
    }
#endif
    value.it_interval.tv_sec = seconds;
    value.it_interval.tv_usec = 0;
    value.it_value.tv_sec = seconds;
    value.it_value.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &value, (struct itimerval*)0)) {
        std::cout << "setitimer failed" << std::endl;
        ParSpike::mpiabort(0);
    }

}



