// ParSpike.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PARSPIKE_H
#define PARSPIKE_H
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <vector>

//must unset for mpi to work
#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END

#include <mpi.h>

#ifndef _spikebuf_size
#define _spikebuf_size 0
#endif
//Globals --


extern  MPI_Comm mpi_comm;
extern  MPI_Comm bbs_comm;

//! SpikePacket_
/*! communication class for sending neuron's ID and time of spike.
 */
typedef struct {
    int gid;
    double spiketime;
} SpikePacket_;
#if _spikebuf_size > 0
typedef struct {
    int nspike;
    int gid[spikebuf_size];
    double spiketime[spikebuf_size];
} SpikeBuffer_;
#endif

//static MPI_Datatype spike_type;


static MPI_Op mpi_pgvts_op;
static void pgvts_op(double* in, double* inout, int* len, MPI_Datatype* dptr);



//! ParSpike: parallel environment variables.
/*! This class encapsulates the parallel environment variables taken from nrnmpi folder in NEURON
 */
class ParSpike
{
public:
    ParSpike(void);


    void init(int under_control, int* pargc, char*** pargv);
    static void terminate();
    static void mpiabort(int errcode);
    static double wtime();
    void make_spikebuf_type();
    void make_spike_type();
    /*inline */void spike_initialize() {
        make_spike_type();
    }
    static int spike_exchange();
    static int spike_exchange_compressed();
    static double mindelay(double m);
    static int int_allmax(int x);
    void int_gather(int* s, int* r, int cnt, int root);
    void int_gatherv(int* s, int scnt, int* r, int* rcnt, int* rdispl, int root);
    void int_alltoallv(int* s, int* scnt, int* sdispl, int* r, int* rcnt, int* rdispl) ;
    void dbl_alltoallv(double* s, int* scnt, int* sdispl, double* r, int* rcnt, int* rdispl);
    static void int_allgather(int* s, int* r, int n);
    void int_allgatherv(int* s, int* r, int* n, int* dspl) ;
    void dbl_allgatherv(double* s, double* r, int* n, int* dspl);
    void dbl_broadcast(double* buf, int cnt, int root);
    void int_broadcast(int* buf, int cnt, int root);
    void char_broadcast(char* buf, int cnt, int root) ;
    int int_sum_reduce(int in, int comm) ;
    void assert_opstep(int opstep, double t, int comm);
    double dbl_allmin(double x, int comm);

    int pgvts_least(double* t, int* op, int* init);

    void send_doubles(double* pd, int cnt, int dest, int tag) ;

    void recv_doubles(double* pd, int cnt, int src, int tag) ;
    void postrecv_doubles(double* pd, int cnt, int src, int tag, void** request);
    void wait(void** request);
    void barrier();
    double dbl_allreduce(double x, int type);
    void dbl_allgather(double* s, double* r, int n) ;


    static std::vector<SpikePacket_> spikeout_;
    static std::vector<SpikePacket_> spikein_;
#if _spikebuf_size > 0
    static std::vector<SpikeBuffer_> spbufout_;
    static std::vector<SpikeBuffer_> spbufin_;
#endif


    static int nout_;
    static int* nin_;
    static int icapacity_;
//  SpikePacket_* spikeout_;
//  SpikePacket_* spikein_;

    static int ag_send_size_;
    static int ag_send_nspike_;
    static int ovfl_capacity_;
    static int ovfl_;
    static std::vector<unsigned char> spfixout_;
    static std::vector<unsigned char> spfixin_;
    static std::vector<unsigned char> spfixin_ovfl_;
    static int localgid_size_;
    static int mpi_use; /* are we using MPI? */
    static int under_mpi_control_;
    static int my_rank; /* rank */
    static int numprocs;
    static int np;
    static int* displs;
    static int* byteovfl; /* for the compressed transfer method */



};

//class ParSpike GlobalSpikeConfig;




#endif  /* PARSPIKE_H */
