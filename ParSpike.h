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


	MPI_Comm mpi_comm;
	MPI_Comm bbs_comm;

/* could convert to typedef pair<int,double>  SpikePacket_*/
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




class ParSpike
{
public:
	ParSpike(void);
	

	void init(int under_control, int* pargc, char*** pargv);
	void terminate();
	void mpiabort(int errcode);
	double wtime();
void make_spikebuf_type();
	void make_spike_type();
	/*inline */void spike_initialize() {
		make_spike_type();
	}
	int spike_exchange();
	int spike_exchange_compressed();
	double mindelay(double m);
	int int_allmax(int x);
	void int_gather(int* s, int* r, int cnt, int root);
	void int_gatherv(int* s, int scnt,
    int* r, int* rcnt, int* rdispl, int root);
	void int_alltoallv(int* s, int* scnt, int* sdispl,
    int* r, int* rcnt, int* rdispl) ;
	void dbl_alltoallv(double* s, int* scnt, int* sdispl,
    double* r, int* rcnt, int* rdispl);
	void int_allgather(int* s, int* r, int n);
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

	SpikePacket_* spikeout_;
	SpikePacket_* spikein_;
#if _spikebuf_size > 0
	SpikeBuffer_* spbufout_;;
	SpikeBuffer_* spbufin_;;
#endif	

	int mpi_use; /* are we using MPI? */
	int nout_; 
	int* nin_;
	int icapacity_;
//	SpikePacket_* spikeout_;
//	SpikePacket_* spikein_;
	int localgid_size_;
	int ag_send_size_;
	int ag_send_nspike_;
	int ovfl_capacity_;
	int ovfl_;
	unsigned char* spfixout_;
	unsigned char* spfixin_;
	unsigned char* spfixin_ovfl_;

	static int under_mpi_control_;
	static int my_rank; /* rank */
	static int numprocs;
	static int np;
	static int* displs;
	static int* byteovfl; /* for the compressed transfer method */



};

//class ParSpike GlobalSpikeConfig;


#endif  /* PARSPIKE_H */
