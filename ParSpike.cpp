// Adapted from NEURON's mpispike.c

#include "ParSpike.h"
#include "BBS2MPI.h"
#include <mpi.h>
#include <errno.h>

ParSpike::ParSpike(void)
{
}

typedef std::map<int,int> MapInt2Int;    

MPI_Comm mpi_comm;
MPI_Comm bbs_comm;
double ParSpike::transfer_wait_=0.0;
int ParSpike::ag_send_size_;
int ParSpike::ag_send_nspike_;
int ParSpike::ovfl_capacity_;
int ParSpike::icapacity_;
int ParSpike::ovfl_;
int ParSpike::nout_ = 0;
int ParSpike::mpi_use = 0;
int ParSpike::numprocs = 1;
int ParSpike::under_mpi_control_ = 1;
int ParSpike::my_rank = 0; /*! MPI rank */
int ParSpike::np;   //automatically set the other static variables to zero
int* ParSpike::displs;
int* ParSpike::nin_;
int* ParSpike::byteovfl;
int ParSpike::localgid_size_ = sizeof(unsigned char);
std::vector<unsigned char> ParSpike::spfixout_;
std::vector<unsigned char> ParSpike::spfixin_;
std::vector<unsigned char> ParSpike::spfixin_ovfl_;
std::vector<SpikePacket_> ParSpike::spikeout_;
std::vector<SpikePacket_> ParSpike::spikein_;
#if _spikebuf_size > 0
std::vector<SpikeBuffer_> ParSpike::spbufout_;;
std::vector<SpikeBuffer_> ParSpike::spbufin_;;
#endif

std::vector<double> ParSpike::targets_;
std::vector<int> ParSpike::sgid2targets_;
//std::list<ConfigBase*> target_pntlist_;
std::vector<double> ParSpike::sources_;
std::vector<int> ParSpike::sgids_;
std::map<int,int> ParSpike::sgid2srcindex_;


static MPI_Datatype spike_type;
extern void bbs_context_wait();

void ParSpike::init(int mpi_control, int* pargc, char*** pargv)
{

    int i, b, flag;
    mpi_use = 1;
    under_mpi_control_ = mpi_control;
    if (under_mpi_control_) {
#if DEBUG == 2

            std::cout << "init: argc=" << *pargc << std::endl;
            for (i = 0; i < *pargc; ++i) {
                std::cout << i << "|" << (*pargv)[i] << "|" << std::endl;
            }
#endif

        MPI_Initialized(&flag);

        if (!flag && MPI_Init(pargc, pargv) != MPI_SUCCESS) {
            std::cout << "MPI_INIT failed" << std::endl;
        }

        MPI_Comm_dup(MPI_COMM_WORLD, &mpi_comm);
    }
    MPI_Comm_dup(mpi_comm, &bbs_comm);
    if (MPI_Comm_rank(mpi_comm, &my_rank) != MPI_SUCCESS) {
        std::cout << "MPI_Comm_rank failed" << std::endl;
    }
    if (MPI_Comm_size(mpi_comm, &numprocs) != MPI_SUCCESS) {
        std::cout << "MPI_Comm_size failed" << std::endl;
    }
    spike_initialize();
    /*begin instrumentation*/

#if DEBUG == 2
    {int i;
        if (my_rank == 0) {
            std::cout << "init: argc=" << *pargc << std::endl;
            for (i = 0; i < *pargc; ++i) {
                std::cout <<  i << " " << (*pargv)[i] << std::endl;
            }
        }
    }
#endif

    if (my_rank == 0) {
        std::cout << "numprocs=" <<  numprocs << std::endl;
    }




}


/*! Create the MPI block of spike packets to be sent/received
 */
void ParSpike::make_spike_type()
{
    SpikePacket_ s;
    int block_lengths[2];
    MPI_Aint displacements[2];
    MPI_Aint addresses[3];
    MPI_Datatype typelist[2];

    typelist[0] = MPI_INT;
    typelist[1] = MPI_DOUBLE;

    block_lengths[0] = block_lengths[1] = 1;

    MPI_Address(&s, &addresses[0]);
    MPI_Address(&(s.gid), &addresses[1]);
    MPI_Address(&(s.spiketime), &addresses[2]);

    displacements[0] = addresses[1] - addresses[0];
    displacements[1] = addresses[2] - addresses[0];

    MPI_Type_struct(2, block_lengths, displacements, typelist, &spike_type);
    MPI_Type_commit(&spike_type);

    MPI_Op_create((MPI_User_function*)pgvts_op, 1, &mpi_pgvts_op);
}

#if nrn_spikebuf_size > 0

static MPI_Datatype spikebuf_type;

void ParSpike::make_spikebuf_type()
{
    SpikeBuffer_ s;
    int block_lengths[3];
    MPI_Aint displacements[3];
    MPI_Aint addresses[4];
    MPI_Datatype typelist[3];

    typelist[0] = MPI_INT;
    typelist[1] = MPI_INT;
    typelist[2] = MPI_DOUBLE;

    block_lengths[0] = 1;
    block_lengths[1] = spikebuf_size;
    block_lengths[2] = spikebuf_size;

    MPI_Address(&s, &addresses[0]);
    MPI_Address(&(s.nspike), &addresses[1]);
    MPI_Address(&(s.gid[0]), &addresses[2]);
    MPI_Address(&(s.spiketime[0]), &addresses[3]);

    displacements[0] = addresses[1] - addresses[0];
    displacements[1] = addresses[2] - addresses[0];
    displacements[2] = addresses[3] - addresses[0];

    MPI_Type_struct(3, block_lengths, displacements, typelist, &spikebuf_type);
    MPI_Type_commit(&spikebuf_type);
}
#endif

double ParSpike::wtime()
{
    return MPI_Wtime();
}

void ParSpike::terminate()
{

    if (under_mpi_control_) {
#if DEBUG == 2
    std::cout << "Terminating: rank " << my_rank << std::endl;
#endif
        MPI_Finalize();
    }
    mpi_use = 0;
#if DEBUG == 2
//      BBS2MPI::checkbufleak();
#endif


}

void ParSpike::mpiabort(int errcode)
{

    int flag;
    MPI_Initialized(&flag);
    if (flag) {
#if DEBUG == 2
    std::cout << "Aborting MPI: rank " << my_rank << std::endl;
#endif        
    MPI_Abort(MPI_COMM_WORLD, errcode);
    } else {
#if DEBUG == 2
    std::cout << "Aborting STD: rank " << my_rank << std::endl;
#endif
	std::abort();
    }

}






/*! Method to Send/Recv  all spikes using MPI_Allgather, taken directly from NEURON.
 */
int ParSpike::spike_exchange()
{
    int i, n, novfl, n1;
    if (!displs) {
        np = numprocs;
        //Fix the memory allocation
        displs = new int[np]; //np*sizeof(int));
        displs[0] = 0;
#if spikebuf_size > 0
        make_spikebuf_type();
#endif
    }
    bbs_context_wait();  /* runs BBSDirectServer::server_->context_wait() this process is the master */
#if spikebuf_size == 0
    MPI_Allgather(&nout_, 1, MPI_INT, nin_, 1, MPI_INT, mpi_comm);
    n = nin_[0];
    for (i = 1; i < np; ++i) {
        displs[i] = n;
        n += nin_[i];
    }
    if (n) {
        if (icapacity_ < n) {
            icapacity_ = n + 10;
            //if (spikein_) delete spikein_;
            spikein_.clear();
            spikein_.resize(icapacity_);
        }
        MPI_Allgatherv(&spikeout_[0], nout_, spike_type, &spikein_[0], nin_, displs, spike_type, mpi_comm);
    }
#else
    MPI_Allgather(&spbufout_[0], 1, spikebuf_type, &spbufin_[0], 1, spikebuf_type, mpi_comm);
    novfl = 0;
    n = spbufin_[0].nspike;
    if (n > spikebuf_size) {
        nin_[0] = n - spikebuf_size;
        novfl += nin_[0];
    } else {
        nin_[0] = 0;
    }
    for (i = 1; i < np; ++i) {
        displs[i] = novfl;
        n1 = spbufin_[i].nspike;
        n += n1;
        if (n1 > spikebuf_size) {
            nin_[i] = n1 - spikebuf_size;
            novfl += nin_[i];
        } else {
            nin_[i] = 0;
        }
    }
    if (novfl) {
        if (icapacity_ < novfl) {
            icapacity_ = novfl + 10;
            spikein_.clear();//if(spikein_) delete [] spikein_;
            spikein_.resize(icapacity_);
        }
        n1 = (nout_ > spikebuf_size) ? nout_ - spikebuf_size : 0;
        MPI_Allgatherv(&spikeout_[0], n1, spike_type, &spikein_[0], nin_, displs, spike_type, mpi_comm);
    }
    ovfl_ = novfl;
#endif
    return n;
}

//! spike_exchange_compressed: comments from original work
/*! The compressed spike format is restricted to the fixed step method and is a sequence of unsigned char.
 * nspike = buf[0]*256 + buf[1]
 * a sequence of spiketime, localgid pairs. There are nspike of them.
 * spiketime is relative to the last transfer time in units of dt.
 * note that this requires a mindelay < 256*dt.
 * localgid is an unsigned int, unsigned short,
 * or unsigned char in size depending on the range and thus takes
 * own byte coding. When the localgid range is smaller than the true
 * gid range, the gid->PreSyn are remapped into
 * hostid specific maps. If there are not many holes, i.e just about every
 * spike from a source machine is delivered to some cell on a
 * target machine, then instead of a hash map, a vector is used.
 * The allgather sends the first part of the buf and the allgatherv buffer
 * sends any overflow.
 */
int ParSpike::spike_exchange_compressed()
{
    int i, novfl, n, ntot, idx, bs, bstot; /* n is #spikes, bs is #byte overflow */
    if (!displs) {
        np = numprocs;

        //todo: change memory
        if (displs) delete displs;
        displs = new int[np];
        displs[0] = 0;
        byteovfl = new int[np]; 
    }
    bbs_context_wait();

    MPI_Allgather(&spfixout_[0], ag_send_size_, MPI_BYTE, &spfixin_[0], ag_send_size_, MPI_BYTE, mpi_comm);
    novfl = 0;
    ntot = 0;
    bstot = 0;
    for (i = 0; i < np; ++i) {
        displs[i] = bstot;
        idx = i * ag_send_size_;
        n = spfixin_[idx++] * 256;
        n += spfixin_[idx++];
        ntot += n;
        nin_[i] = n;
        if (n > ag_send_nspike_) {
            bs = 2 + n * (1 + localgid_size_) - ag_send_size_;
            byteovfl[i] = bs;
            bstot += bs;
            novfl += n - ag_send_nspike_;
        } else {
            byteovfl[i] = 0;
        }
    }
    if (novfl) {
        if (ovfl_capacity_ < novfl) {
            ovfl_capacity_ = novfl + 10;
            spfixin_ovfl_.clear();
            spfixin_ovfl_.resize(ovfl_capacity_ *(1 + localgid_size_)); 
        }
        bs = byteovfl[my_rank];
        /*
        note that the spfixout_ buffer is one since the overflow
        is contiguous to the first part. But the spfixin_ovfl_ is
        completely separate from the spfixin_ since the latter
        dynamically changes its size during a run.
        */
        MPI_Allgatherv(&spfixout_[ag_send_size_], bs, MPI_BYTE, &spfixin_ovfl_[0], byteovfl, displs, MPI_BYTE, mpi_comm);
    }
    ovfl_ = novfl;
    return ntot;
}

double ParSpike::mindelay(double m)
{
    double result;
    if (!mpi_use) {
        return m;
    }
    bbs_context_wait();
    MPI_Allreduce(&m, &result, 1, MPI_DOUBLE, MPI_MIN, mpi_comm);
    return result;
}


int ParSpike::int_allmax(int x)
{
    int result;
    if (numprocs < 2) {
        return x;
    }
    bbs_context_wait();
    MPI_Allreduce(&x, &result, 1, MPI_INT, MPI_MAX, mpi_comm);
    return result;
}

//! int_gather interface to MPI_Gather
void ParSpike::int_gather(int* s, int* r, int cnt, int root)
{
    MPI_Gather(s, cnt, MPI_INT, r, cnt, MPI_INT, root, mpi_comm);
}

//! int_gatherv interface to MPI_Gatherv
void ParSpike::int_gatherv(int* s, int scnt,
                           int* r, int* rcnt, int* rdispl, int root)
{
    MPI_Gatherv(s, scnt, MPI_INT, r, rcnt, rdispl, MPI_INT, root, mpi_comm);
}

//! int_alltoallv interface to MPI_Alltoallv with MPI_INT
void ParSpike::int_alltoallv(int* s, int* scnt, int* sdispl,
                             int* r, int* rcnt, int* rdispl)
{
    MPI_Alltoallv(s, scnt, sdispl, MPI_INT,
                  r, rcnt, rdispl, MPI_INT, mpi_comm);
}

//! dbl_alltoallv interface to MPI_Alltoallv with MPI_DOUBLES
void ParSpike::dbl_alltoallv(double* s, int* scnt, int* sdispl,
                             double* r, int* rcnt, int* rdispl)
{
    MPI_Alltoallv(s, scnt, sdispl, MPI_DOUBLE,
                  r, rcnt, rdispl, MPI_DOUBLE, mpi_comm);
}

/* following are for the partrans */

void ParSpike::int_allgather(int* s, int* r, int n)
{
    MPI_Allgather(s, n,  MPI_INT, r, n, MPI_INT, mpi_comm);
}

void ParSpike::int_allgatherv(int* s, int* r, int* n, int* dspl)
{
    MPI_Allgatherv(s, n[my_rank],  MPI_INT,
                   r, n, dspl, MPI_INT, mpi_comm);
}

void ParSpike::dbl_allgatherv(double* s, double* r, int* n, int* dspl)
{
    MPI_Allgatherv(s, n[my_rank],  MPI_DOUBLE,
                   r, n, dspl, MPI_DOUBLE, mpi_comm);
}

void ParSpike::dbl_broadcast(double* buf, int cnt, int root)
{
    MPI_Bcast(buf, cnt,  MPI_DOUBLE, root, mpi_comm);
}

void ParSpike::int_broadcast(int* buf, int cnt, int root)
{
#if DEBUG ==2 
std::cout << my_rank << " int_broadcast " <<cnt << " buf[0]=" (my_rank == root ? buf[0]:-1)<< std::endl;
#endif
    MPI_Bcast(buf, cnt,  MPI_INT, root, mpi_comm);
}

void ParSpike::char_broadcast(char* buf, int cnt, int root)
{
    MPI_Bcast(buf, cnt,  MPI_CHAR, root, mpi_comm);
}

int ParSpike::int_sum_reduce(int in, int comm)
{
    int result;
    MPI_Allreduce(&in, &result, 1, MPI_INT, MPI_SUM, mpi_comm);
    return result;
}

void ParSpike::assert_opstep(int opstep, double t, int comm)
{
    /* all machines in comm should have same opstep and same t. */
    double buf[2];
    if (numprocs < 2) {
        return;
    }
    buf[0] = (double)opstep;
    buf[1] = t;
    MPI_Bcast(buf, 2, MPI_DOUBLE, 0, mpi_comm);
    if (opstep != (int)buf[0]  || t != buf[1]) {
        std::cout << my_rank << " opstep=" << opstep << " "
                  << (int)buf[0] << "  t=" << t << " t-troot="
                  << t - buf[1] << std::endl;
        std::cerr << "ParSpike::assert_opstep failed"; std::cerr.flush();
    }
}

double ParSpike::dbl_allmin(double x, int comm)
{
    double result;
    if (numprocs < 2) {
        return x;
    }
    MPI_Allreduce(&x, &result, 1, MPI_DOUBLE, MPI_MIN, mpi_comm);
    return result;
}

static void pgvts_op(double* in, double* inout, int* len, MPI_Datatype* dptr)
{
    int i;
    assert(*dptr == MPI_DOUBLE);
    assert(*len == 4);
    if (in[0] <= inout[0]) {
        if (in[0] < inout[0]) {
            for (i = 0; i < 4; ++i) {
                inout[i] = in[i];
            }
        } else if (in[3] < inout[3]) {
            // NetParEvent done last, init next to last.
            for (i = 0; i < 4; ++i) {
                inout[i] = in[i];
            }
        }
    }
}

int ParSpike::pgvts_least(double* t, int* op, int* init)
{
    double ibuf[4], obuf[4];
    ibuf[0] = *t;
    ibuf[1] = (double)(*op);
    ibuf[2] = (double)(*init);
    ibuf[3] = (double)my_rank;
    MPI_Allreduce(ibuf, obuf, 4, MPI_DOUBLE, mpi_pgvts_op, mpi_comm);
    *t = obuf[0];
    *op = (int)obuf[1];
    *init = (int)obuf[2];
    if (my_rank == (int)obuf[3]) {
        return 1;
    }
    return 0;
}


void ParSpike::setup_transfer() 
{

#ifdef DEBUG
  std::cout << "setup_transfer" << std::endl;
#endif
	alloclists();
	//is_setup_ = true;

	// send this machine source count to all other machines and get the
	// source counts for all machines. This allows us to create the
	// proper incoming source buffer and make the outgoing source pointer
	// point to the right spot in that.
	if (!srccnt_) {
		srccnt_ = new int[numprocs];
		srcdspl_ = new int[numprocs];
	}
	srccnt_[my_rank] = sources_.size();
	if (numprocs > 1) {
	  int_allgather(srccnt_ + my_rank, srccnt_, 1);
		errno = 0;
	}
	
	// allocate the source buffer
	int i, j;
	src_buf_size_ = 0;
	for (i=0; i < numprocs; ++i) {
		srcdspl_[i] = src_buf_size_;
		src_buf_size_ += srccnt_[i];
	}
	if (incoming_source_buf_) {
		delete [] incoming_source_buf_;
		incoming_source_buf_ = 0;
		outgoing_source_buf_ = 0;
	}
	if (src_buf_size_ == 0) {
	  //	v_transfer_ = 0;
		return;
	}
	int osb = srcdspl_[my_rank];
	incoming_source_buf_ = new double[src_buf_size_];
	outgoing_source_buf_ = incoming_source_buf_ + osb;

	// send this machines list of sgids (in source buf order) to all
	// machines and receive corresponding lists from all machines. This
	// allows us to create the source buffer index to target list
	// (by finding the index into the sgid to target list).
	int* sgid = new int[src_buf_size_];
	for (i = 0, j = osb; i < sources_.size(); ++i, ++j) {
	  sgid[j] = sgids_[i];
	}
	if (numprocs > 1) {
		int_allgatherv(sgid + osb, sgid, srccnt_, srcdspl_);
	}
	errno = 0;
	
	if (s2t_index_) { delete [] s2t_index_; }
	s2t_index_ = new int[targets_.size()];
	std::map<int,int> mi2;
//printf("srcbufsize=%d\n", src_buf_size_);
	for (i = 0; i < src_buf_size_; ++i) {
	  if (mi2[sgid[i]]==j) {
			std::cout << my_rank << ":ParSpike: multiple instances of source gid: " << sgid[i] << std::endl;
		}
		mi2[sgid[i]] = i;
	}
	for (i=0; i < targets_.size(); ++i) {
	  assert(mi2[sgid2targets_[i]] == s2t_index_[i]);
	}
	delete [] sgid;
//	delete mi2;
	//	v_transfer_ = var_transfer;
	//nrn_mk_transfer_thread_data_ = mk_ttd;
	//if (!v_structure_change) {
	//	mk_ttd();
	//}
}




//! Barrier interface to MPI_Barrier
/*! Halts processors until they all reach this point
 */ 
void ParSpike::barrier()
{
#if DEBUG == 2
  if (my_rank != 0)  std::cout << "Halting on MPI_barrier()"<< std::endl;
#endif
  if (MPI_Barrier(mpi_comm) != MPI_SUCCESS ) std::cout << "MPI_Barrier failure" << std::endl;
}

double ParSpike::dbl_allreduce(double x, int type)
{
    double result;
    MPI_Op t;
    if (numprocs < 2) {
        return x;
    }
    if (type == 1) {
        t = MPI_SUM;
    } else if (type == 2) {
        t = MPI_MAX;
    } else {
        t = MPI_MIN;
    }
    MPI_Allreduce(&x, &result, 1, MPI_DOUBLE, t, mpi_comm);
    return result;
}


/* following for splitcell.cpp transfer */
void ParSpike::send_doubles(double* pd, int cnt, int dest, int tag)
{
    MPI_Send(pd, cnt, MPI_DOUBLE, dest, tag, mpi_comm);
}

void ParSpike::recv_doubles(double* pd, int cnt, int src, int tag)
{
    MPI_Status status;
    MPI_Recv(pd, cnt, MPI_DOUBLE, src, tag, mpi_comm, &status);
}

void ParSpike::postrecv_doubles(double* pd, int cnt, int src, int tag, void** request)
{
    MPI_Irecv(pd, cnt, MPI_DOUBLE, src, tag, mpi_comm, (MPI_Request*)request);
}

void ParSpike::wait(void** request)
{
    MPI_Status status;
    MPI_Wait((MPI_Request*)request, &status);
}

void ParSpike::dbl_allgather(double* s, double* r, int n)
{
    MPI_Allgather(s, n,  MPI_DOUBLE, r, n, MPI_DOUBLE, mpi_comm);
}

#if BGPDMA
static MPI_Comm bgp_comm;

void ParSpike::bgp_comm()
{
    if (!bgp_comm) {
        MPI_Comm_dup(MPI_COMM_WORLD, &bgp_comm);
    }
}

void ParSpike::bgp_multisend(SpikePacket_* spk, int n, int* hosts)
{
    int i;
    MPI_Request r;
    MPI_Status status;
    for (i = 0; i < n; ++i) {
        MPI_Isend(spk, 1, spike_type, hosts[i], 1, bgp_comm, &r);
//std::cout << "%d multisend n=%d i=%d host=%d gid=%d t=%g\n",
//my_rank, n, i, hosts[i], spk->gid, spk->spiketime);
        MPI_Request_free(&r);
//      MPI_Wait(&r, &status);
    }
}

int ParSpike::bgp_single_advance(SpikePacket_* spk)
{
    int flag = 0;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, 1, bgp_comm, &flag, &status);
    if (flag) {
        MPI_Recv(spk, 1, spike_type, MPI_ANY_SOURCE, 1, bgp_comm, &status);
//std::cout << "%d advance receive gid=%d t=%g\n",
//my_rank, spk->gid, spk->spiketime);
    }
    return flag;
}

static int iii;
int bgp_conserve(int nsend, int nrecv)
{
    int tcnts[2];
    tcnts[0] = nsend - nrecv;
    MPI_Allreduce(tcnts, tcnts + 1, 1, MPI_INT, MPI_SUM, bgp_comm);
//std::cout << "%d conserve %d %d %d\n", my_rank, nsend, nrecv, tcnts[1]);
    return tcnts[1];
}

#endif /*BGPDMA*/



void ParSpike::alloclists() {
  //if (!targets_) {
	  /*      	  *targets_ = new std::list< double >;
	  *sgid2targets_ = new  std::list<int>;
	  // target_pntlist_ = new std::list<ConfigBase;
	  *sources_ = new std::list<double>;
	  *sgids_ = new std::list<int>;
	  sgid2srcindex_ = new  std::map<int,int>;
	  */ 
incoming_source_buf_ = new double;
	  outgoing_source_buf_ = new double;
	  s2t_index_ = new int;

 targets_.clear(); targets_.reserve(100);
		//	target_pntlist_.capacity(100);
	  sgid2targets_.clear(); sgid2targets_.reserve(100);
	  sources_.clear(); sources_.reserve(100);
	  sgids_.clear(); sgids_.reserve(100);
	  sgid2srcindex_.clear();//sgid2srcindex_.reserve(256);
	  
	  //	}
	//	v_transfer_ = var_transfer;
}

void ParSpike::nrn_partrans_clear() {
  //	if (!targets_) { return; }
	//	v_transfer_ = 0;
	sgid2srcindex_.clear();
	sgids_.clear();
	sources_.clear();
	sgid2targets_.clear();
	targets_.clear();
	//	delete target_pntlist_;
	// targets_ = 0;
	//	rm_ttd();
	//nrn_mk_transfer_thread_data_ = 0;
}




void ParSpike::var_transfer() {

//	fprintf(xxxfile, "%g\n", t);
/*	if (nrn_nthread > 1) {
		// threads and nhost is tangential to the main purpose and
		// can be thought about further if needed
		assert(numprocs == 1);
		// for threads we do direct transfers under the assumption
		// that v is being transferred and they were set in a
		// previous multithread job. For the fixed step method this
		// call is from nonvint which in the same thread job as update
		// and that is the case even with multisplit. So we really
		// need to break the job between update and nonvint. Too bad.
		// For global cvode, things are ok except if the source voltage
		// is at a zero area node since nocap_v_part2 is a part
		// of this job and in fact the v does not get updated til
		// the next job in nocap_v_part3. Again, too bad. But it is
		// quite ambiguous, stability wise,
		// to have a gap junction in a zero area node, anyway, since
		// the system is then truly a DAE.
		// For now we presume we have dealt with these matters and
		// do the transfer.

				assert(n_transfer_thread_data_ == nrn_nthread);
		TransferThreadData& ttd = transfer_thread_data_[_nt->d];
		for (int i = 0; i < ttd.cnt; ++i) {
			*targets_->item(ttd.ti[i]) = *sources_->item(ttd.si[i]);
		}
		return;
		
	}
*/
	int i, n = sources_.size();
	for (i=0; i < n; ++i) {
	  outgoing_source_buf_[i] = sources_[i];
	}
#if DEBUG == 2
for (i=0; i < 10; ++i) {
	printf("outgoing %d %g\n", i, outgoing_source_buf_[i]);
}
#endif
#if PARALLELSIM
 if (numprocs > 1) {
		double wt = wtime();
		dbl_allgatherv(outgoing_source_buf_, incoming_source_buf_,
			srccnt_, srcdspl_);
		//	transfer_wait_ += wtime() - wt;
		errno = 0;
	}
#endif
	n = targets_.size();
	for (i=0; i < n; ++i) {
	  targets_[i] = incoming_source_buf_[s2t_index_[i]];
	}
#if 0
for (i=0; i < 10; ++i) {
	printf("targets %d %d %g\n", i, s2t_index_[i],  targets_[i]);
}
#endif
}

#if 0
#include <signal.h>
#include <sys/time.h>
#include <section.h>

static double told;
static struct itimerval value;
#if !defined(BLUEGENE)
static struct sigaction act, oact;
#endif


void ParSpike::timed_out(int sig) {
#ifdef DEBUG
  std::cout << "timed_out told = " << told << "  t= "<< t << std::endl;
#endif
	if (_t == told) { /* nothing has been accomplished since last signal*/
	  std::cout << "nrn_timeout t="<< _t<<std::endl;
		ParSpike::abort(0);
	}
	told = _t;
}

void ParSpike::timeout(int seconds) {
	if (my_rank != 0) { return; }
#if BLUEGENE
	if (seconds) {
		told = nrn_threads->_t;
		signal(SIGALRM, timed_out);
	}else{
		signal(SIGALRM, SIG_DFL);
	}
#else
	if (seconds) {
		told = nrn_threads->_t;
		act.sa_handler = timed_out;
		act.sa_flags = SA_RESTART;
		if(sigaction(SIGALRM, &act, &oact)) {
		  std::cout<< "sigaction failed"<< std::endl;
			mpiabort(0);
		}
	}else{
		sigaction(SIGALRM, &oact, (struct sigaction*)0);
	}
#endif
	value.it_interval.tv_sec = seconds;
	value.it_interval.tv_usec = 0;
	value.it_value.tv_sec = seconds;
	value.it_value.tv_usec = 0;
	if(setitimer(ITIMER_REAL, &value, (struct itimerval*)0)) {
	  std::cout<< "setitimer failed" <<std::endl;
	  mpiabort(0);
	}
	
}

#else

void ParSpike::timeout(int err){}

#endif
