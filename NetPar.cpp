#include "BBS.h"
#include "NetPar.h"



#undef MD
#define MD 2147483648.

class PreSyn;

// hash table where buckets are binary search maps
declareNrnHash(Gid2PreSyn, int, PreSyn*)
implementNrnHash(Gid2PreSyn, int, PreSyn*)

NetPar::NetPar(void)
{
}


/*
NetParEvent::NetParEvent(){
	wx_ = ws_ = 0.;
	ithread_ = -1;
}
NetParEvent::~NetParEvent(){
}

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

    if (nrnmpi_numprocs > 0 && nt->id == 0) {
  	nrn_spike_exchange();
	wx_ += wt_;
	ws_ += wt1_;
   }

	send(tt, nc, nt);
}
void NetParEvent::pgvts_deliver(double tt, NetCvode* nc){
	assert(0);
	deliver(tt, nc, 0);
}

void NetParEvent::pr(const char* m, double tt, NetCvode* nc){
	printf("%s NetParEvent %d t=%.15g tt-t=%g\n", m, ithread_, tt, tt - nrn_threads[ithread_]._t);
}

DiscreteEvent* NetParEvent::savestate_save(){
	//pr("savestate_save", 0, net_cvode_instance);
	NetParEvent* npe =  new NetParEvent();
	npe->ithread_ = ithread_;
	return npe;
}

DiscreteEvent* NetParEvent::savestate_read(FILE* f){
	int i;
	assert(fscanf(f, "%d", &i) == 1);
	//printf("NetParEvent::savestate_read %d\n", i);
	NetParEvent* npe = new NetParEvent();
	npe->ithread_ = i;
	return npe;
}

void NetParEvent::savestate_write(FILE* f){
	//pr("savestate_write", 0, net_cvode_instance);
	fprintf(f, "%d %d\n", NetParEventType, ithread_);
}

void NetParEvent::savestate_restore(double tt, NetCvode* nc){

	if (use_compress_) {
		t_exchange_ = t;
	}
	if (ithread_ == 0) {
		//npe_->pr("savestate_restore", tt, nc);
		for (int i=0; i < nrn_nthread; ++i) {
			nc->event(tt, npe_+i, nrn_threads + i);
		}
	}

}
*/


void NetPar::nrn_outputevent(unsigned char localgid, double firetime) {
	if (!active_) { return; }
	nout_++;
	int i = idxout_;
	idxout_ += 2;
	if (idxout_ >= spfixout_capacity_) {
		spfixout_capacity_ *= 2;
		spfixout_.resize(spfixout_capacity_);
	}
	spfixout_[i++] = (unsigned char)((firetime - t_exchange_)*dt1_ + .5);
	spfixout_[i] = localgid;
//printf("%d idx=%d lgid=%d firetime=%g t_exchange_=%g [0]=%d [1]=%d\n", nrnmpi_myid, i, (int)localgid, firetime, t_exchange_, (int)spfixout_[i-1], (int)spfixout_[i]);
}

/*
void nrn2ncs_outputevent(int gid, double firetime) {
	if (!active_) { return; }
    if (use_compress_) {
	nout_++;
	int i = idxout_;
	idxout_ += 1 + localgid_size_;
	if (idxout_ >= spfixout_capacity_) {
		spfixout_capacity_ *= 2;
		spfixout_.resize(spfixout_capacity_);
	}
//printf("%d nrnncs_outputevent %d %.20g %.20g %d\n", nrnmpi_myid, gid, firetime, t_exchange_,
//(int)((unsigned char)((firetime - t_exchange_)*dt1_ + .5)));
	spfixout_[i++] = (unsigned char)((firetime - t_exchange_)*dt1_ + .5);
//printf("%d idx=%d firetime=%g t_exchange_=%g spfixout=%d\n", nrnmpi_myid, i, firetime, t_exchange_, (int)spfixout_[i-1]);
	sppk(spfixout_+i, gid);
//printf("%d idx=%d gid=%d spupk=%d\n", nrnmpi_myid, i, gid, spupk(spfixout_+i));
    }else{
#if nrn_spikebuf_size == 0
	int i = nout_++;
	if (i >= ocapacity_) {
		ocapacity_ *= 2;
		spikeout_.resize(ocapacity_);
	}		
//printf("%d cell %d in slot %d fired at %g\n", nrnmpi_myid, gid, i, firetime);
	spikeout_[i].gid = gid;
	spikeout_[i].spiketime = firetime;
#else
	int i = nout_++;
	if (i >= nrn_spikebuf_size) {
		i -= nrn_spikebuf_size;
		if (i >= ocapacity_) {
			ocapacity_ *= 2;
			spikeout_.resize(ocapacity_);
		}		
		spikeout_[i].gid = gid;
		spikeout_[i].spiketime = firetime;
	}else{
		spbufout_->gid[i] = gid;
		spbufout_->spiketime[i] = firetime;
	}
#endif
    }
//printf("%d cell %d in slot %d fired at %g\n", nrnmpi_myid, gid, i, firetime);
}
*/

int NetPar::nrn_need_npe() {
	
	int b = 0;
	if (active_) { b = 1; }
	if (nrn_use_selfqueue_) { b = 1; }
	if (nrn_nthread > 1) { b = 1; }
	if (b) {
		if (last_maxstep_arg_ == 0) {
			last_maxstep_arg_ =   100.;
		}
		set_mindelay(last_maxstep_arg_);
	}else{
		if (npe_) {
			delete [] npe_;
			npe_ = nil;
			n_npe_ = 0;
		}
	}
	return b;
}

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

void NetPar::spike_exchange_init() {
//printf("nrn_spike_exchange_init\n");
	if (!nrn_need_npe()) { return; }
//	if (!active_ && !nrn_use_selfqueue_) { return; }
	alloc_space();
//printf("nrnmpi_use=%d active=%d\n", nrnmpi_use, active_);
	calc_actual_mindelay();	
	usable_mindelay_ = mindelay_;
	if (cvode_active_ == 0 && nrn_nthread > 1) {
		usable_mindelay_ -= dt;
	}
	if ((usable_mindelay_ < 1e-9) || (cvode_active_ == 0 && usable_mindelay_ < dt)) {
		if (nrnmpi_myid == 0) {
			std::cerr << "usable mindelay is 0 (or less than dt for fixed step method) \n";
		}else{
			return;
		}
	}

	if (n_npe_ != nrn_nthread) {
		if (npe_) { delete [] npe_; }
		npe_ = new NetParEvent[nrn_nthread];
		n_npe_ = nrn_nthread;
	}
	for (int i = 0; i < nrn_nthread; ++i) {
		npe_[i].ithread_ = i;
		npe_[i].wx_ = 0.;
		npe_[i].ws_ = 0.;
		npe_[i].send(t, net_cvode_instance, nrn_threads + i);
	}

    if (use_compress_) {
	idxout_ = 2;
	t_exchange_ = t;
	dt1_ = 1./dt;
	usable_mindelay_ = floor(mindelay_ * dt1_ + 1e-9) * dt;
	assert (usable_mindelay_ >= dt && (usable_mindelay_ * dt1_) < 255);
    }else{
#if nrn_spikebuf_size > 0
	if (spbufout_) {
		spbufout_->nspike = 0;
	}
#endif
    }
	nout_ = 0;
	nsend_ = nsendmax_ = nrecv_ = nrecv_useful_ = 0;

	//if (nrnmpi_myid == 0){printf("usable_mindelay_ = %g\n", usable_mindelay_);}
}


void NetPar::spike_exchange() {
	if (!active_) { return; }

	if (use_compress_) { nrn_spike_exchange_compressed(); return; }
	double wt;
	int i, n;
#if NRNSTAT
	nsend_ += nout_;
	if (nsendmax_ < nout_) { nsendmax_ = nout_; }
#endif
#if nrn_spikebuf_size > 0
	spbufout_->nspike = nout_;
#endif
	wt = ParSpike::wtime();
	n = ParSpike::spike_exchange();
	wt_ = ParSpike::wtime() - wt;
	wt = ParSpike::wtime();
	errno = 0;
//if (n > 0) {
//printf("%d nrn_spike_exchange sent %d received %d\n", nrnmpi_myid, nout_, n);
//}
	nout_ = 0;
	if (n == 0) {
#if NRNSTAT
		if (max_histogram_) { max_histogram_[0] += 1.; }
#endif
		return;
	}
#if NRNSTAT
	nrecv_ += n;
	if (max_histogram_) {
		int mx = 0;
		if (n > 0) {
			for (i=nrnmpi_numprocs-1 ; i >= 0; --i) {
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
		int ms = max_histogram_.size()-1;
		mx = (mx < ms) ? mx : ms;
		vector_vec(max_histogram_)[mx] += 1.;
	}
#endif // NRNSTAT
#if nrn_spikebuf_size > 0
	for (i = 0; i < nrnmpi_numprocs; ++i) {
		int j;
		int nn = spbufin_[i].nspike;
		if (nn > nrn_spikebuf_size) { nn = nrn_spikebuf_size; }
		for (j=0; j < nn; ++j) {
			PreSyn* ps;
			if (gid2in_->find(spbufin_[i].gid[j], ps)) {
				ps->send(spbufin_[i].spiketime[j], net_cvode_instance, nrn_threads);
#if NRNSTAT
				++nrecv_useful_;
#endif
			}
		}
	}
	n = ovfl_;
#endif // nrn_spikebuf_size > 0
	for (i = 0; i < n; ++i) {
		PreSyn* ps;
		if (gid2in_->find(spikein_[i].gid, ps)) {
			ps->send(spikein_[i].spiketime, net_cvode_instance, nrn_threads);
#if NRNSTAT
			++nrecv_useful_;
#endif
		}
	}
	wt1_ = nrnmpi_wtime() - wt;
}
		
void nrn_spike_exchange_compressed() {
	if (!active_) { return; }
	assert(!cvode_active_);
	double wt;
	int i, n, idx;
#if NRNSTAT
	nsend_ += nout_;
	if (nsendmax_ < nout_) { nsendmax_ = nout_; }
#endif
	assert(nout_ < 0x10000);
	spfixout_[1] = (unsigned char)(nout_ & 0xff);
	spfixout_[0] = (unsigned char)(nout_>>8);

	wt = nrnmpi_wtime();
	n = nrnmpi_spike_exchange_compressed();
	wt_ = nrnmpi_wtime() - wt;
	wt = nrnmpi_wtime();
	errno = 0;
//if (n > 0) {
//printf("%d nrn_spike_exchange sent %d received %d\n", nrnmpi_myid, nout_, n);
//}
	nout_ = 0;
	idxout_ = 2;
	if (n == 0) {
#if NRNSTAT
		if (max_histogram_) { vector_vec(max_histogram_)[0] += 1.; }
#endif
		t_exchange_ = nrn_threads->_t;
		return;
	}
#if NRNSTAT
	nrecv_ += n;
	if (max_histogram_) {
		int mx = 0;
		if (n > 0) {
			for (i=nrnmpi_numprocs-1 ; i >= 0; --i) {
				if (mx < nin_[i]) {
					mx = nin_[i];
				}
			}
		}
		int ms = vector_capacity(max_histogram_)-1;
		mx = (mx < ms) ? mx : ms;
		vector_vec(max_histogram_)[mx] += 1.;
	}
#endif // NRNSTAT
    if (nrn_use_localgid_) {
	int idxov = 0;
	for (i = 0; i < nrnmpi_numprocs; ++i) {
		int j, nnn;
		int nn = nin_[i];
	    if (nn) {
		if (i == nrnmpi_myid) { // skip but may need to increment idxov.
			if (nn > ag_send_nspike_) {
				idxov += (nn - ag_send_nspike_)*(1 + localgid_size_);
			}
			continue;
		}
		Gid2PreSyn* gps = localmaps_[i];
		if (nn > ag_send_nspike_) {
			nnn = ag_send_nspike_;
		}else{
			nnn = nn;
		}
		idx = 2 + i*ag_send_size_;
		for (j=0; j < nnn; ++j) {
			// order is (firetime,gid) pairs.
			double firetime = spfixin_[idx++]*dt + t_exchange_;
			int lgid = (int)spfixin_[idx];
			idx += localgid_size_;
			PreSyn* ps;
			if (gps->find(lgid, ps)) {
				ps->send(firetime + 1e-10, net_cvode_instance, nrn_threads);
#if NRNSTAT
				++nrecv_useful_;
#endif
			}
		}
		for ( ; j < nn; ++j) {
			double firetime = spfixin_ovfl_[idxov++]*dt + t_exchange_;
			int lgid = (int)spfixin_ovfl_[idxov];
			idxov += localgid_size_;
			PreSyn* ps;
			if (gps->find(lgid, ps)) {
				ps->send(firetime+1e-10, net_cvode_instance, nrn_threads);
#if NRNSTAT
				++nrecv_useful_;
#endif
			}
		}
	    }
	}
    }else{
	for (i = 0; i < nrnmpi_numprocs; ++i) {
		int j;
		int nn = nin_[i];
		if (nn > ag_send_nspike_) { nn = ag_send_nspike_; }
		idx = 2 + i*ag_send_size_;
		for (j=0; j < nn; ++j) {
			// order is (firetime,gid) pairs.
			double firetime = spfixin_[idx++]*dt + t_exchange_;
			int gid = spupk(spfixin_ + idx);
			idx += localgid_size_;
			PreSyn* ps;
			if (gid2in_->find(gid, ps)) {
				ps->send(firetime+1e-10, net_cvode_instance, nrn_threads);
#if NRNSTAT
				++nrecv_useful_;
#endif
			}
		}
	}
	n = ovfl_;
	idx = 0;
	for (i = 0; i < n; ++i) {
		double firetime = spfixin_ovfl_[idx++]*dt + t_exchange_;
		int gid = spupk(spfixin_ovfl_ + idx);
		idx += localgid_size_;
		PreSyn* ps;
		if (gid2in_->find(gid, ps)) {
			ps->send(firetime+1e-10, net_cvode_instance, nrn_threads);
#if NRNSTAT
			++nrecv_useful_;
#endif
		}
	}
    }
	t_exchange_ = nrn_threads->_t;
	wt1_ = nrnmpi_wtime() - wt;
}


void mk_localgid_rep() {
	int i, j, k;
	PreSyn* ps;

	// how many gids are there on this machine
	// and can they be compressed into one byte
	int ngid = 0;
	NrnHashIterate(Gid2PreSyn, gid2out_, PreSyn*, ps) {
		if (ps->output_index_ >= 0) {
			++ngid;
		}
	}}}
	int ngidmax = nrnmpi_int_allmax(ngid);
	if (ngidmax >= 256) {
		//do not compress
		return;
	}
	localgid_size_ = sizeof(unsigned char);
	nrn_use_localgid_ = true;

	// allocate Allgather receive buffer (send is the nrnmpi_myid one)
	int* rbuf = new int[nrnmpi_numprocs*(ngidmax + 1)];
	int* sbuf = rbuf + my_rank*(ngidmax + 1);

	sbuf[0] = ngid;
	++sbuf;
	ngid = 0;
	// define the local gid and fill with the gids on this machine
	NrnHashIterate(Gid2PreSyn, gid2out_, PreSyn*, ps) {
		if (ps->output_index_ >= 0) {
			ps->localgid_ = (unsigned char)ngid;
			sbuf[ngid] = ps->output_index_;
			++ngid;
		}
	}}}
	--sbuf;

	// exchange everything
	int_allgather(sbuf, rbuf, ngidmax+1);
	errno = 0;

	// create the maps
	// there is a lot of potential for efficiency here. i.e. use of
	// perfect hash functions, or even simple Vectors.
	localmaps_ = new Gid2PreSyn*[numprocs];
	localmaps_[nrnmpi_myid] = 0;
	for (i = 0; i < nrnmpi_numprocs; ++i) if (i != nrnmpi_myid) {
		// how many do we need?
		sbuf = rbuf + i*(ngidmax + 1);
		ngid = *(sbuf++); // at most
		// of which we actually use...
		for (k=0, j=0; k < ngid; ++k) {
			if (gid2in_ && gid2in_->find(int(sbuf[k]), ps)) {
				++j;
			}
		}
		// oh well. there is probably a rational way to choose but...
		localmaps_[i] = new Gid2PreSyn((j > 19) ? 19 : j+1);
	}

	// fill in the maps
	for (i = 0; i < nrnmpi_numprocs; ++i) if (i != nrnmpi_myid) {
		sbuf = rbuf + i*(ngidmax + 1);
		ngid = *(sbuf++);
		for (k=0; k < ngid; ++k) {
			if (gid2in_ && gid2in_->find(int(sbuf[k]), ps)) {
				(*localmaps_[i])[k] = ps;
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
void nrn_fake_fire(int gid, double spiketime, int fake_out) {
	assert(gid2in_);
	PreSyn* ps;
	if (gid2in_->find(gid, ps)) {
		assert(ps);
//printf("nrn_fake_fire %d %g\n", gid, spiketime);
		ps->send(spiketime, net_cvode_instance, nrn_threads);
#if NRNSTAT
		++nrecv_useful_;
#endif
	}else if (fake_out && gid2out_->find(gid, ps)) {
		assert(ps);
//printf("nrn_fake_fire fake_out %d %g\n", gid, spiketime);
		ps->send(spiketime, net_cvode_instance, nrn_threads);
#if NRNSTAT
		++nrecv_useful_;
#endif
	}

}

static void alloc_space() {
	if (!gid2out_) {
		netcon_sym_ = hoc_lookup("NetCon");
		gid2out_ = new Gid2PreSyn(211);
		gid2in_ = new Gid2PreSyn(2311);

		ocapacity_  = 100;
		spikeout_.resize(ocapacity_);
		icapacity_  = 100;
		spikein_.resize(icapacity_);
		if (nin_) delete nin_[];
		nin_ = new int[nrnmpi_numprocs];
#if nrn_spikebuf_size > 0
spbufout_.resize(1);
spbufin_.resize(nrnmpi_numprocs);
#endif

	}
}

void BBS::set_gid2node(int gid, int nid) {
	alloc_space();

	if (nid == nrnmpi_myid) {

	{
#endif
//printf("gid %d defined on %d\n", gid, nrnmpi_myid);
		PreSyn* ps;
		assert(!(gid2in_->find(gid, ps)));
		(*gid2out_)[gid] = nil;
//		gid2out_->insert(pair<const int, PreSyn*>(gid, nil));
	}
}

void nrnmpi_gid_clear() {
	nrn_partrans_clear();
#if PARANEURON
	nrnmpi_split_clear();
#endif
	nrnmpi_multisplit_clear();
	if (!gid2out_) { return; }
	PreSyn* ps, *psi;
	NrnHashIterate(Gid2PreSyn, gid2out_, PreSyn*, ps) {
		if (ps && !gid2in_->find(ps->gid_, psi)) {
			ps->gid_ = -1;
			ps->output_index_ = -1;
			if (ps->dil_.count() == 0) {
				delete ps;
			}
		}
	}}}
	NrnHashIterate(Gid2PreSyn, gid2in_, PreSyn*, ps) {
		ps->gid_ = -1;
		ps->output_index_ = -1;
		if (ps->dil_.count() == 0) {
			delete ps;
		}
	}}}
	int i;
	for (i = gid2out_->size_ - 1; i >= 0; --i) {
		gid2out_->at(i).clear();
	}
	for (i = gid2in_->size_ - 1; i >= 0; --i) {
		gid2in_->at(i).clear();
	}
}

int BBS::gid_exists(int gid) {
	PreSyn* ps;
	alloc_space();
	if (gid2out_->find(gid, ps)) {
//printf("%d gid %d exists\n", nrnmpi_myid, gid);
		if (ps) {
			return (ps->output_index_ >= 0 ? 3 : 2);
		}else{
			return 1;
		}
	}
	return 0;
}

double BBS::threshold() {
	int gid = int(chkarg(1, 0., MD));
	PreSyn* ps;
	assert(gid2out_->find(gid, ps));
	assert(ps);
	if (ifarg(2)) {
		ps->threshold_ = *getarg(2);
	}
	return ps->threshold_;
}

void BBS::cell() {
	int gid = int(chkarg(1, 0., MD));
	PreSyn* ps;
	assert(gid2out_->find(gid, ps));
	Object* ob = *hoc_objgetarg(2);
	if (!ob || ob->ctemplate != netcon_sym_->u.ctemplate) {
		check_obj_type(ob, "NetCon");
	}
	NetCon* nc = (NetCon*)ob->u.this_pointer;
	ps = nc->src_;
//printf("%d cell %d %s\n", nrnmpi_myid, gid, hoc_object_name(ps->ssrc_ ? ps->ssrc_->prop->dparam[6].obj : ps->osrc_));
	(*gid2out_)[gid] = ps;
	ps->gid_ = gid;
	if (ifarg(3) && !chkarg(3, 0., 1.)) {
		ps->output_index_ = -2; //prevents destruction of PreSyn
	}else{
		ps->output_index_ = gid;
	}
}

void BBS::outputcell(int gid) {
	PreSyn* ps;
	assert(gid2out_->find(gid, ps));
	assert(ps);
	ps->output_index_ = gid;
	ps->gid_ = gid;
}

void BBS::spike_record(int gid, IvocVect* spikevec, IvocVect* gidvec) {
	PreSyn* ps;
	assert(gid2out_->find(gid, ps));
	assert(ps);
	ps->record(spikevec, gidvec, gid);
}

Object** BBS::gid2obj(int gid) {
	Object* cell = 0;
//printf("%d gid2obj gid=%d\n", nrnmpi_myid, gid);
	PreSyn* ps;
	assert(gid2out_->find(gid, ps));
//printf(" found\n");
	assert(ps);
	cell = ps->ssrc_ ? ps->ssrc_->prop->dparam[6].obj : ps->osrc_;
//printf(" return %s\n", hoc_object_name(cell));
	return hoc_temp_objptr(cell);
}

Object** BBS::gid2cell(int gid) {
	Object* cell = 0;
//printf("%d gid2obj gid=%d\n", nrnmpi_myid, gid);
	PreSyn* ps;
	assert(gid2out_->find(gid, ps));
//printf(" found\n");
	assert(ps);
	if (ps->ssrc_) {
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
//printf(" return %s\n", hoc_object_name(cell));
	return hoc_temp_objptr(cell);
}

Object** BBS::gid_connect(int gid) {
	Object* target = *hoc_objgetarg(2);
	if (!is_point_process(target)) {
		hoc_execerror("arg 2 must be a point process", 0);
	}
	alloc_space();
	PreSyn* ps;
	if (gid2out_->find(gid, ps)) {
		// the gid is owned by this machine so connect directly
		assert(ps);
	}else if (gid2in_->find(gid, ps)) {
		// the gid stub already exists
//printf("%d connect %s from already existing %d\n", nrnmpi_myid, hoc_object_name(target), gid);
	}else{
//printf("%d connect %s from new PreSyn for %d\n", nrnmpi_myid, hoc_object_name(target), gid);
		ps = new PreSyn(nil, nil, nil);
		net_cvode_instance->psl_append(ps);
		(*gid2in_)[gid] = ps;
		ps->gid_ = gid;
	}
	NetCon* nc;
	Object** po;
	if (ifarg(3)) {
		po = hoc_objgetarg(3);
		if (!*po || (*po)->ctemplate != netcon_sym_->u.ctemplate) {
			check_obj_type(*po, "NetCon");
		}
		nc = (NetCon*)((*po)->u.this_pointer);
		if (nc->target_ != ob2pntproc(target)) {
			hoc_execerror("target is different from 3rd arg NetCon target", 0);
		}
		nc->replace_src(ps);
	}else{
		nc = new NetCon(ps, target);
		po = hoc_temp_objvar(netcon_sym_, nc);
		nc->obj_ = *po;
	}
	return po;
}

void BBS::netpar_solve(double tstop) {

	double mt, md;
	tstopunset;
	if (cvode_active_) {
		mt = 1e-9 ; md = mindelay_;
	}else{
		mt = dt ; md = mindelay_ - 1e-10;
	}
	if (md < mt) {
		if (nrnmpi_myid == 0) {
			hoc_execerror("mindelay is 0", "(or less than dt for fixed step method)");
		}else{
			return;
		}
	}
	double wt;

	nrn_timeout(20);
	wt = nrnmpi_wtime();
	if (cvode_active_) {
		ncs2nrn_integrate(tstop);
	}else{
		ncs2nrn_integrate(tstop+1e-11);
	}
	impl_->integ_time_ += nrnmpi_wtime() - wt;
	impl_->integ_time_ -= (npe_ ? (npe_[0].wx_ + npe_[0].ws_) : 0.);

	nrn_spike_exchange();

	nrn_timeout(0);
	impl_->wait_time_ += wt_;
	impl_->send_time_ += wt1_;
	if (npe_) {
		impl_->wait_time_ += npe_[0].wx_;
		impl_->send_time_ += npe_[0].ws_;
		npe_[0].wx_ = npe_[0].ws_ = 0.;
	};
//printf("%d netpar_solve exit t=%g tstop=%g mindelay_=%g\n",nrnmpi_myid, t, tstop, mindelay_);

	tstopunset;
}

static double set_mindelay(double maxdelay) {
	double mindelay = maxdelay;
	last_maxstep_arg_ = maxdelay;
    if (nrn_use_selfqueue_ || net_cvode_instance->localstep() || nrn_nthread > 1 ) {
	hoc_Item* q;
	if (net_cvode_instance->psl_) ITERATE(q, net_cvode_instance->psl_) {
		PreSyn* ps = (PreSyn*)VOIDITM(q);
		double md = ps->mindelay();
		if (mindelay > md) {
			mindelay = md;
		}
	}
    }
#if NRNMPI
    else{
	NrnHashIterate(Gid2PreSyn, gid2in_, PreSyn*, ps) {
		double md = ps->mindelay();
		if (mindelay > md) {
			mindelay = md;
		}
	}}}
    }
	if (nrnmpi_use) {active_ = 1;}
	if (use_compress_) {
		if (mindelay/dt > 255) {
			mindelay = 255*dt;
		}
	}

//printf("%d netpar_mindelay local %g now calling nrnmpi_mindelay\n", nrnmpi_myid, mindelay);
//	double st = time();
	mindelay_ = nrnmpi_mindelay(mindelay);
	min_interprocessor_delay_ = mindelay_;
//	add_wait_time(st);
//printf("%d local min=%g  global min=%g\n", nrnmpi_myid, mindelay, mindelay_);
	if (mindelay_ < 1e-9 && nrn_use_selfqueue_) {
		nrn_use_selfqueue_ = 0;
		double od = mindelay_;
		mindelay = set_mindelay(maxdelay);
		if (nrnmpi_myid == 0) {
printf("Notice: The global minimum NetCon delay is %g, so turned off the cvode.queue_mode\n", od);
printf("   use_self_queue option. The interprocessor minimum NetCon delay is %g\n", mindelay);
		}
	}
#if BGPDMA
	if (use_bgpdma_) {
		bgp_dma_setup();
	}
#endif
	errno = 0;
	return mindelay;
#else
	mindelay_ = mindelay;
	min_interprocessor_delay_ = mindelay_;
	return mindelay;
#endif //NRNMPI
}

double BBS::netpar_mindelay(double maxdelay) {
	return set_mindelay(maxdelay);
}

void BBS::netpar_spanning_statistics(int* nsend, int* nsendmax, int* nrecv, int* nrecv_useful) {
#if NRNMPI
	*nsend = nsend_;
	*nsendmax = nsendmax_;
	*nrecv = nrecv_;
	*nrecv_useful = nrecv_useful_;
#endif
}

// unfortunately, ivocvect.h conflicts with STL
std::vector<double>* BBS::netpar_max_histogram(std::vector<double>* mh) {

	std::vector<double>* h = max_histogram_;
	if (max_histogram_) {
		max_histogram_ = nil;
	}
	if (mh) {
		max_histogram_ = mh;
	}
	return h;

}

int nrnmpi_spike_compress(int nspike, boolean gid_compress, int xchng_meth) {

	if (nrnmpi_numprocs < 2) { return 0; }
#if BGPDMA
	use_bgpdma_ = (xchng_meth == 1) ? 1 : 0;
	if (nrnmpi_myid == 0) {printf("use_bgpdma_ = %d\n", use_bgpdma_);}
#endif
	if (nspike >= 0) {
		ag_send_nspike_ = 0;
		if (spfixout_) { spfixout_.resize(0); spfixout_ = 0; }
		if (spfixin_) { spfixin_.resize(0); spfixin_ = 0; }
		if (spfixin_ovfl_) { spfixin_ovfl_.resize(0); spfixin_ovfl_ = 0; }
		if (localmaps_) {
			for (int i=0; i < nrnmpi_numprocs; ++i) if (i != nrnmpi_myid) {
				if (localmaps_[i]) { delete localmaps_[i]; }
			}
			delete [] localmaps_;
			localmaps_ = 0;
		}
	}
	if (nspike == 0) { // turn off
		use_compress_ = false;
		nrn_use_localgid_ = false;
	}else if (nspike > 0) { // turn on
		if (cvode_active_) {
if (nrnmpi_myid == 0) {printf("ParallelContext.spike_compress cannot be used with cvode active", 0);}
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
			if (!nrn_use_localgid_ && nrnmpi_myid == 0) {
printf("Notice: gid compression did not succeed. Probably more than 255 cells on one cpu.\n");
			}
		}
		if (!nrn_use_localgid_) {
			localgid_size_ = sizeof(unsigned int);
		}
		ag_send_size_ = 2 + ag_send_nspike_*(1 + localgid_size_);
		spfixout_capacity_ = ag_send_size_ + 50*(1 + localgid_size_);
		spfixout_.resize(spfixout_capacity_); 
		spfixin_.resize(nrnmpi_numprocs);
		ovfl_capacity_ = 100;
		spfixin_ovfl_.resize(ovfl_capacity_);
	}
	return ag_send_nspike_;

}
