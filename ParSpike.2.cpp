// from mpispike.c

#include "ParSpike.2.h"
#include <mpi.h>

ParSpike::ParSpike(void)
{
}

MPI::Intracomm mpi_comm;
MPI::Intracomm bbs_comm;
MPI::Op ParSpike::mpi_pgvts_op = 0;
int ParSpike::ag_send_size_;
int ParSpike::ag_send_nspike_;
int ParSpike::ovfl_capacity_;
int ParSpike::icapacity_;
int ParSpike::ovfl_;
int ParSpike::nout_ = 0;
int ParSpike::mpi_use = 0;
int ParSpike::numprocs = 1;
int ParSpike::under_mpi_control_ = 1;
int ParSpike::my_rank = 0; /* rank */
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

static MPI::Datatype spike_type;
extern void bbs_context_wait();

void ParSpike::init(int mpi_control, int& pargc, char**& pargv)
{


    int i, b, flag;
    mpi_use = 1;
    under_mpi_control_ = mpi_control;
    if (under_mpi_control_) {
#ifdef DEBUG
            std::cout << "init: argc=" << pargc << std::endl;
            for (i = 0; i < pargc; ++i) {
                std::cout << i << "|" << (pargv)[i] << "|" << std::endl;
            }
            std::cout << "Starting MPI::Init" << std::endl;
#endif

        MPI::Init(pargc, pargv);
        flag = MPI::Is_initialized();

        if (flag == MPI::SUCCESS) {
            std::cout << "MPI::INIT failed, flag = "<<flag << std::endl;
        }else {
            std::cout << "MPI::Init succeeded, flag = "<<flag << std::endl;
	}
        mpi_comm = MPI::COMM_WORLD;
        //MPI_Comm_dup(MPI_COMM_WORLD, &mpi_comm);
    }
    bbs_comm = mpi_comm.Dup();
    my_rank = mpi_comm.Get_rank();
    std::cout << "MPI_Comm::Get_rank my_rank = " << my_rank << std::endl;
    numprocs = mpi_comm.Get_size();
    std::cout << "MPI_Comm::Get_size, numprocs = " << numprocs << std::endl;

    spike_initialize();
    /*begin instrumentation*/

#ifdef DEBUG
    
        if (my_rank == 0) {
            std::cout << "init2: argc=" << pargc << std::endl;
            for (i = 0; i < pargc; ++i) {
                std::cout << i << " |" << (pargv)[i]  << "| " << std::endl;
            }
        }
    
#endif

    if (my_rank == 0) {
        std::cout << "numprocs=" <<  numprocs << std::endl;
    }




}



void ParSpike::make_spike_type()
{
#ifdef DEBUG
    std::cout << my_rank << " : make_spike_type"  << std::endl;
#endif

    SpikePacket_ s;
    int block_lengths[2];
    MPI::Aint displacements[2];
    MPI::Aint addresses[3];
    MPI::Datatype typelist[2];

    typelist[0] = MPI::INT;
    typelist[1] = MPI::DOUBLE;

    block_lengths[0] = block_lengths[1] = 1;

    addresses[0] = MPI::Get_address(&s);
    addresses[1] = MPI::Get_address(&(s.gid));
    addresses[2] = MPI::Get_address(&(s.spiketime));

    displacements[0] = addresses[1] - addresses[0];
    displacements[1] = addresses[2] - addresses[0];

    spike_type.Create_struct(2, block_lengths, displacements, typelist);
    spike_type.Commit();
    mpi_pgvts_op.Init((MPI::User_function*)pgvts_op, 1);
    //MPI_Op_create((MPI_User_function*)pgvts_op, 1, &mpi_pgvts_op);
}

#if nrn_spikebuf_size > 0

//static MPI_Datatype spikebuf_type;
static MPI::Datatype spikebuf_type;

void ParSpike::make_spikebuf_type()
{
#ifdef DEBUG
    std::cout << my_rank << " : make_spikebuf_type"  << std::endl;
#endif

    SpikeBuffer_ s;
    int block_lengths[3];
    MPI::Aint displacements[3];
    MPI::Aint addresses[4];
    MPI::Datatype typelist[3];

    typelist[0] = MPI::INT;
    typelist[1] = MPI::INT;
    typelist[2] = MPI::DOUBLE;

    block_lengths[0] = 1;
    block_lengths[1] = spikebuf_size;
    block_lengths[2] = spikebuf_size;

    addresses[0] = MPI::Get_address(&s);
    addresses[1] = MPI::Get_address(&(s.nspike));
    addresses[2] = MPI::Get_address(&(s.gid[0]));
    addresses[3] = MPI::Get_address(&(s.spiketime[0]));

    displacements[0] = addresses[1] - addresses[0];
    displacements[1] = addresses[2] - addresses[0];
    displacements[2] = addresses[3] - addresses[0];

    spikebuf_type.Create_struct(3, block_lengths, displacements, typelist);
    spikebuf_type.Commit();
}
#endif

double ParSpike::wtime()
{
    return MPI::Wtime();
}

void ParSpike::terminate()
{

#ifdef DEBUG
    std::cout << my_rank << " : terminate"  << std::endl;
#endif
    if (under_mpi_control_) {
        MPI::Finalize();
    }
    mpi_use = 0;
#ifdef DEBUG
    //checkbufleak();
#endif


}

void ParSpike::mpiabort(int errcode)
{

    bool flag = MPI::Is_initialized();
    if (flag) {
        mpi_comm.Abort(errcode);
    } else {
        std::abort();
    }

}






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
// runs BBSDirectServer::server_->context_wait() this process is the master 
    bbs_context_wait();  

#if spikebuf_size == 0
    mpi_comm.Allgather(&nout_, 1, MPI::INT, nin_, 1, MPI::INT);
    n = nin_[0];
    for (i = 1; i < np; ++i) {
        displs[i] = n;
        n += nin_[i];
    }
    if (n) {
        if (icapacity_ < n) {
            icapacity_ = n + 10;
            spikein_.clear();
            spikein_.resize(icapacity_);
        }
        mpi_comm.Allgatherv(&spikeout_[0], nout_, spike_type, &spikein_[0], nin_, displs, spike_type);
    }
#else
    mpi_comm.Allgather(&spbufout_[0], 1, spikebuf_type, &spbufin_[0], 1, spikebuf_type);
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
            spikein_.clear();
            spikein_.resize(icapacity_);
        }
        n1 = (nout_ > spikebuf_size) ? nout_ - spikebuf_size : 0;
        mpi_comm.Allgatherv(&spikeout_[0], n1, spike_type, &spikein_[0], nin_, displs, spike_type);
    }
    ovfl_ = novfl;
#endif
    return n;
}


/*
The compressed spike format is restricted to the fixed step method and is
a sequence of unsigned char.
nspike = buf[0]*256 + buf[1]
a sequence of spiketime, localgid pairs. There are nspike of them.
    spiketime is relative to the last transfer time in units of dt.
    note that this requires a mindelay < 256*dt.
    localgid is an unsigned int, unsigned short,
    or unsigned char in size depending on the range and thus takes
    4, 2, or 1 byte respectively. To be machine independent we do our
    own byte coding. When the localgid range is smaller than the true
    gid range, the gid->PreSyn are remapped into
    hostid specific maps. If there are not many holes, i.e just about every
    spike from a source machine is delivered to some cell on a
    target machine, then instead of a hash map, a vector is used.
The allgather sends the first part of the buf and the allgatherv buffer
sends any overflow.
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

    mpi_comm.Allgather(&spfixout_[0], ag_send_size_, MPI::BYTE, &spfixin_[0], ag_send_size_, MPI::BYTE);
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
        mpi_comm.Allgatherv(&spfixout_[ag_send_size_], bs, MPI::BYTE, &spfixin_ovfl_[0], byteovfl, displs, MPI::BYTE);
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
    mpi_comm.Allreduce(&m, &result, 1, MPI::DOUBLE, MPI::MIN);
    return result;
}


int ParSpike::int_allmax(int x)
{
    int result;
    if (numprocs < 2) {
        return x;
    }
    bbs_context_wait();
    mpi_comm.Allreduce(&x, &result, 1, MPI::INT, MPI::MAX);
    return result;
}

void ParSpike::int_gather(int* s, int* r, int cnt, int root)
{
    mpi_comm.Gather(s, cnt, MPI::INT, r, cnt, MPI::INT, root);

}

void ParSpike::int_gatherv(int* s, int scnt,
                           int* r, int* rcnt, int* rdispl, int root)
{
    mpi_comm.Gatherv(s, scnt, MPI::INT, r, rcnt, rdispl, MPI::INT, root);
}

void ParSpike::int_alltoallv(int* s, int* scnt, int* sdispl,
                             int* r, int* rcnt, int* rdispl)
{
    mpi_comm.Alltoallv(s, scnt, sdispl, MPI::INT,
                       r, rcnt, rdispl, MPI::INT);
}

void ParSpike::dbl_alltoallv(double* s, int* scnt, int* sdispl,
                             double* r, int* rcnt, int* rdispl)
{
    mpi_comm.Alltoallv(s, scnt, sdispl, MPI::DOUBLE,
                       r, rcnt, rdispl, MPI::DOUBLE);
}

/* following are for the partrans */

void ParSpike::int_allgather(int* s, int* r, int n)
{
    mpi_comm.Allgather(s, n,  MPI::INT, r, n, MPI::INT);
}

void ParSpike::int_allgatherv(int* s, int* r, int* n, int* dspl)
{
    mpi_comm.Allgatherv(s, n[my_rank],  MPI::INT,
                        r, n, dspl, MPI::INT);
}

void ParSpike::dbl_allgatherv(double* s, double* r, int* n, int* dspl)
{
    mpi_comm.Allgatherv(s, n[my_rank],  MPI::DOUBLE,
                        r, n, dspl, MPI::DOUBLE);
}

void ParSpike::dbl_broadcast(double* buf, int cnt, int root)
{
    mpi_comm.Bcast(buf, cnt,  MPI::DOUBLE, root);
}

void ParSpike::int_broadcast(int* buf, int cnt, int root)
{
//std::cout << "%d int_broadcast %d buf[0]=%d\n", my_rank, cnt, my_rank == root ? buf[0]: -1);
    mpi_comm.Bcast(buf, cnt,  MPI::INT, root);
}

void ParSpike::char_broadcast(char* buf, int cnt, int root)
{ 
    mpi_comm.Bcast(buf, cnt,  MPI::CHAR, root);
}

int ParSpike::int_sum_reduce(int in, int comm)
{
    int result;
    mpi_comm.Allreduce(&in, &result, 1, MPI::INT, MPI::SUM);

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
    mpi_comm.Bcast(buf, 2, MPI::DOUBLE, 0);

    if (opstep != (int)buf[0]  || t != buf[1]) {
        std::cout << my_rank << " opstep=" << opstep << " "
                  << (int)buf[0] << "  t=" << t << " t-troot="
                  << t - buf[1] << std::endl;
        std::cerr << "mpi_assert_opstep failed"; std::cerr.flush();
    }
}

double ParSpike::dbl_allmin(double x, int comm)
{
    double result;
    if (numprocs < 2) {
        return x;
    }
    mpi_comm.Allreduce(&x, &result, 1, MPI::DOUBLE, MPI::MIN);
    return result;
}

//static void pgvts_op(double* in, double* inout, int* len, MPI_Datatype* dptr){
static void pgvts_op(const void* in_, void* inout_, int len, const MPI::Datatype& dptr)
{
    int i;
    double * in = (double*)in_;
    double* inout = (double*) inout_;
    //assert(*dptr == MPI_DOUBLE);
    assert(dptr == MPI::DOUBLE);
    //assert(*len == 4);
    assert(len == 4);
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
    mpi_comm.Allreduce(ibuf, obuf, 4, MPI::DOUBLE, mpi_pgvts_op);
    *t = obuf[0];
    *op = (int)obuf[1];
    *init = (int)obuf[2];
    if (my_rank == (int)obuf[3]) {
        return 1;
    }
    return 0;
}

/* following for splitcell.cpp transfer */
void ParSpike::send_doubles(double* pd, int cnt, int dest, int tag)
{
    mpi_comm.Send(pd, cnt, MPI::DOUBLE, dest, tag);
}

void ParSpike::recv_doubles(double* pd, int cnt, int src, int tag)
{
    MPI::Status status;
    mpi_comm.Recv((void*)pd, cnt, MPI::DOUBLE, src, tag, status);
}

void ParSpike::postrecv_doubles(double* pd, int cnt, int src, int tag, void** request_)
{
    MPI::Request* request = (MPI::Request*)request_;
    *request = mpi_comm.Irecv(pd, cnt, MPI::DOUBLE, src, tag);
}

void ParSpike::wait(void** request_)
{
    MPI::Status status;
    MPI::Request* request = (MPI::Request*)request_;
    request->Wait(status);
}

void ParSpike::barrier()
{
    mpi_comm.Barrier();
}

double ParSpike::dbl_allreduce(double x, int type)
{
    double result;
    MPI::Op t;
    if (numprocs < 2) {
        return x;
    }
    if (type == 1) {
        t = MPI::SUM;
    } else if (type == 2) {
        t = MPI::MAX;
    } else {
        t = MPI::MIN;
    }
    mpi_comm.Allreduce(&x, &result, 1, MPI::DOUBLE, t);
    return result;
}

void ParSpike::dbl_allgather(double* s, double* r, int n)
{
    mpi_comm.Allgather(s, n,  MPI::DOUBLE, r, n, MPI::DOUBLE);

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


