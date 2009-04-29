
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "BBS2SpikeSim.h"
#include <mpi.h>

#define debugleak 0
#define debug 0

extern MPI_Comm bbs_comm;

#if debugleak
static int BBS2SpikeSim::bufcnt_;
#endif

/* we want to find the key easily and I am assuming that all MPI
 implementations allow one to start unpacking from a particular position.
 Therefore we give space for the key position at the beginning and
 an int always takes up the same space at position 0.
 Now I regret not forcing the key to come first at the user level.
*/

#define my_MPI_INT 0
#define my_MPI_DOUBLE 1
#define my_MPI_CHAR 2
#define my_MPI_PACKED 3

static MPI_Datatype mytypes[] = {MPI_INT, MPI_DOUBLE, MPI_CHAR, MPI_PACKED};


BBS2SpikeSim::BBS2SpikeSim()
{

}

 void BBS2SpikeSim::unpack(void* buf, int count, int my_datatype, bbsmpibuf* r, const char* errmes) {
	int type[2];
	assert(r);
#if debug
//printf("%d unpack upkpos=%d pkposition=%d keypos=%d size=%d\n",
//  mpi_rank, r->upkpos, r->pkposition, r->keypos, r->size);
#endif
assert(r->upkpos >= 0 && r->size >= r->upkpos);
	MPI_Unpack(&(r->buf[0]), r->size, &r->upkpos, type, 2, MPI_INT, bbs_comm); 
#if debug
//printf("%d unpack r=%lx size=%d upkpos=%d type[0]=%d datatype=%d  type[1]=%d  count=%d\n", mpi_rank, (long)r, r->size, r->upkpos, type[0], my_datatype, type[1], count);
#endif
if (type[0] != my_datatype || type[1] != count) {
//printf("%d unpack size=%d upkpos=%d type[0]=%d   datatype=%d  type[1]=%d  count=%d\n", mpi_rank, r->size, r->upkpos, type[0], my_datatype, type[1], count);
}
	assert(type[0] == my_datatype);
	assert(type[1] == count);
	MPI_Unpack(&(r->buf[0]), r->size, &r->upkpos, buf, count, mytypes[my_datatype], bbs_comm);
}

void BBS2SpikeSim::upkbegin(bbsmpibuf* r) {
	int type;
	int p;
#if debug
//printf("%d BBS2SpikeSim::upkbegin %lx (preunpack upkpos=%d keypos=%d)\n", mpi_rank, (long)r, r->upkpos, r->keypos);
#endif
assert(r  && r->size > 0);
	r->upkpos = 0;
	MPI_Unpack(&r->buf[0], r->size, &r->upkpos,
		&p, 1, MPI_INT, bbs_comm);
if (p > r->size) {
//printf("\n %d BBS2SpikeSim::upkbegin keypos=%d size=%d\n", mpi_rank, p, r->size);
}
assert(p <= r->size);
	MPI_Unpack(&(r->buf[0]), r->size, &p, &type, 1, MPI_INT, bbs_comm);
#if debug
//printf("%d BBS2SpikeSim::upkbegin type=%d keypos=%d\n", mpi_rank, type, p);
#endif
	assert(type == 0);
	r->keypos = p;
}

char* BBS2SpikeSim::getkey(bbsmpibuf* r) {
	char* s;
	int type;
	type = r->upkpos;
	r->upkpos = r->keypos;
#if debug
//printf("%d BBS2SpikeSim::getkey %lx keypos=%d\n", mpi_rank, (long)r, r->keypos);
#endif
	s = BBS2SpikeSim::upkstr(r);
	assert(r->pkposition == 0 || r->pkposition == r->upkpos);
	r->pkposition = r->upkpos;
	r->upkpos = type;
#if debug
//printf("getkey return %s\n", s);
#endif
	return s;
}

int BBS2SpikeSim::getid(bbsmpibuf* r) {
	int i, type;
	type = r->upkpos;
	r->upkpos = r->keypos;
#if debug
//printf("%d BBS2SpikeSim::getid %lx keypos=%d\n", mpi_rank, (long)r, r->keypos);
#endif
	i = BBS2SpikeSim::upkint(r);
	r->upkpos = type;
#if debug
//printf("getid return %d\n", i);
#endif
	return i;
}

int BBS2SpikeSim::upkint(bbsmpibuf* r) {
	int i;
	unpack(&i, 1, my_MPI_INT, r, "upkint");
	return i;
}

double BBS2SpikeSim::upkdouble(bbsmpibuf* r) {
	double x;
	unpack(&x, 1, my_MPI_DOUBLE, r, "upkdouble");
	return x;
}

void BBS2SpikeSim::upkvec(int n, double* x, bbsmpibuf* r) {
	unpack(x, n, my_MPI_DOUBLE, r, "upkvec");
}

char* BBS2SpikeSim::upkstr(bbsmpibuf* r) {
	int len;
	char* s;
	unpack(&len, 1, my_MPI_INT, r, "upkstr length");
	s = new char[len+1];
	unpack(s, len, my_MPI_CHAR, r, "upkstr string");
	s[len] = '\0';
	return s;
}

static void resize(bbsmpibuf* r, int size) {
	int newsize;
	if (r->size < size) {
		newsize = (size/64)*64 + 128;
		r->buf.resize(newsize); 
		r->size = newsize;
	}
}

void BBS2SpikeSim::pkbegin(bbsmpibuf* r) {
	int type;
	r->pkposition = 0;
	type = 0;
#if debug
//printf("%d BBS2SpikeSim::pkbegin %lx size=%d pkposition=%d\n", mpi_rank, (long)r, r->size, r->pkposition);
#endif
	MPI_Pack(&type, 1, MPI_INT, &(r->buf[0]), r->size, &r->pkposition, bbs_comm);
}

void BBS2SpikeSim::enddata(bbsmpibuf* r) {
	int p, type, isize, oldsize;
	p = r->pkposition;
	type = 0;
#if debug
//printf("%d BBS2SpikeSim::enddata %lx size=%d pkposition=%d\n", mpi_rank, (long)r, r->size, p);
#endif
	MPI_Pack_size(1, MPI_INT, bbs_comm, &isize);
oldsize = r->size;
	resize(r, r->pkposition + isize);
#if debug
if (oldsize < r->pkposition + isize) {
	//printf("%d %lx need %d more. end up with total of %d\n", mpi_rank, (long)r, isize, r->size);
}
#endif
	MPI_Pack(&type, 1, MPI_INT, &(r->buf[0]), r->size, &r->pkposition, bbs_comm);
#if debug
//printf("%d BBS2SpikeSim::enddata buf=%lx size=%d pkposition=%d\n", mpi_rank, r->buf, r->size, r->pkposition);
#endif
	MPI_Pack(&p, 1, MPI_INT, &(r->buf[0]), r->size, &type, bbs_comm);
#if debug
//printf("%d after BBS2SpikeSim::enddata, %d was packed at beginning and 0 was packed before %d\n", mpi_rank, p, r->pkposition);
#endif
}

static void pack(void* inbuf, int incount, int my_datatype, bbsmpibuf* r, const char* e) {
	int type[2];
	int dsize, isize, oldsize;
#if debug
//printf("%d pack %lx count=%d type=%d outbuf-%lx pkposition=%d %s\n", mpi_rank, (long)r, incount, my_datatype, r->buf, r->pkposition, e);
#endif
	MPI_Pack_size(incount, mytypes[my_datatype], bbs_comm, &dsize);
	MPI_Pack_size(2, MPI_INT, bbs_comm, &isize);
oldsize = r->size;
	resize(r, r->pkposition + dsize + isize);
#if debug
if (oldsize < r->pkposition + dsize + isize) {
	//printf("%d %lx need %d more. end up with total of %d\n", mpi_rank, (long)r, dsize+isize, r->size);
}
#endif
	type[0] = my_datatype;  type[1] = incount;
	MPI_Pack(type, 2, MPI_INT, &(r->buf[0]), r->size, &r->pkposition, bbs_comm);
	MPI_Pack(inbuf, incount, mytypes[my_datatype], &(r->buf[0]), r->size, &r->pkposition, bbs_comm);
#if debug
//printf("%d pack done pkposition=%d\n", mpi_rank, r->pkposition);
#endif
}

void BBS2SpikeSim::pkint(int i, bbsmpibuf* r) {
	int ii;
	ii = i;
	pack(&ii, 1, my_MPI_INT, r, "pkint");
}

void BBS2SpikeSim::pkdouble(double x, bbsmpibuf* r) {
	double xx;
	xx = x;
	pack(&xx, 1, my_MPI_DOUBLE, r, "pkdouble");
}

void BBS2SpikeSim::pkvec(int n, double* x, bbsmpibuf* r) {
	pack(x, n, my_MPI_DOUBLE, r, "pkvec");
}

void BBS2SpikeSim::pkstr(const char* s, bbsmpibuf* r) {
	int len;
	len = strlen(s);
	pack(&len, 1, my_MPI_INT, r, "pkstr length");
	pack((char*)s, len, my_MPI_CHAR, r, "pkstr string");
}

void BBS2SpikeSim::bbssend(int dest, int tag, bbsmpibuf* r) {
#if debug
//printf("%d BBS2SpikeSim::bbssend %lx dest=%d tag=%d size=%d\n", mpi_rank, (long)r, dest, tag, (r)?r->upkpos:0);
#endif
	if (r) {
		assert( r->keypos <= r->size);
		MPI_Send(&(r->buf[0]), r->pkposition, MPI_PACKED, dest, tag, bbs_comm);
	}else{
		MPI_Send(NULL, 0, MPI_PACKED, dest, tag, bbs_comm);
	}
	errno = 0;
#if debug
//printf("%d return from send\n", mpi_rank);
#endif
}

int BBS2SpikeSim::bbsrecv(int source, bbsmpibuf* r) {
	MPI_Status status;
	int size;
	if (source == -1) {
		source = MPI_ANY_SOURCE;
	}
#if debug
//printf("%d BBS2SpikeSim::bbsrecv %lx\n", mpi_rank, (long)r);
#endif
	MPI_Probe(source, MPI_ANY_TAG, bbs_comm, &status);
	MPI_Get_count(&status, MPI_PACKED, &size);
#if debug
//printf("%d BBS2SpikeSim::bbsrecv probe size=%d source=%d tag=%d\n", mpi_rank, size, status.MPI_SOURCE, status.MPI_TAG);
#endif
	resize(r, size);
	MPI_Recv(&(r->buf[0]), r->size, MPI_PACKED, source, MPI_ANY_TAG, bbs_comm, &status);
	errno = 0;
	return status.MPI_TAG;
}

int BBS2SpikeSim::bbssendrecv(int dest, int tag, bbsmpibuf* s, bbsmpibuf* r) {
	int size, itag, source;
	int msgtag;
	MPI_Status status;
#if debug
//printf("%d BBS2SpikeSim::bbssendrecv dest=%d tag=%d\n", mpi_rank, dest, tag);
#endif
	if (!BBS2SpikeSim::iprobe(&size, &itag, &source) || source != dest) {
#if debug
//printf("%d BBS2SpikeSim::bbssendrecv nothing available so send\n", mpi_rank);
#endif
		BBS2SpikeSim::bbssend(dest, tag, s);
	}
		return BBS2SpikeSim::bbsrecv(dest, r);
}

int BBS2SpikeSim::iprobe(int* size, int* tag, int* source) {
	int flag = 0;
	MPI_Status status;
	MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, bbs_comm, &flag, &status);
	if (flag) {
		if (source) *source = status.MPI_SOURCE;
		if (tag) *tag = status.MPI_TAG;
		if (size) MPI_Get_count(&status, MPI_PACKED, size);
	}
	return flag;
}

bbsmpibuf* BBS2SpikeSim::newbuf(int size) {
	
	bbsmpibuf* buf= new bbsmpibuf;
#if debug
//printf("%d BBS2SpikeSim::newbuf %lx\n", mpi_rank, (long)buf);
#endif
//	buf->buf = (char*)0;
	buf->buf.resize(size);
	buf->size = size;
	buf->pkposition = 0;
	buf->upkpos = 0;
	buf->keypos = 0;
	buf->refcount = 0;
#if debugleak
	++BBS2SpikeSim::bufcnt_;
#endif
	return buf;	
}

void BBS2SpikeSim::copy(bbsmpibuf* dest, bbsmpibuf* src){
	int i;
	resize(dest, src->size);
//	for (i=0; i < src->size; ++i) {
//		dest->buf[i] = src->buf[i];
//	}
	std::copy(src->buf.begin(), src->buf.end(), dest->buf.begin());
	dest->pkposition = src->pkposition;
	dest->upkpos = src->upkpos;
	dest->keypos = src->keypos;
}

void BBS2SpikeSim::free(bbsmpibuf* buf){
#if debug
//printf("%d BBS2SpikeSim::free %lx\n", mpi_rank, (long)buf);
#endif
//	if (buf->buf) {
//		free(buf->buf);		/* STL vector automatic deletion */
//	}
//	free(buf);				/* boost::shared_ptr auto deletion */
	delete buf;
#if debugleak
	--BBS2SpikeSim::bufcnt_;
#endif
}

void BBS2SpikeSim::ref(bbsmpibuf* buf) {
	assert(buf);
	buf->refcount += 1;
}

void BBS2SpikeSim::unref(bbsmpibuf* buf) {
	if (buf) {
		--buf->refcount;
		if (buf->refcount <= 0) {
			BBS2SpikeSim::free(buf);
		}
	}
}

#if debugleak
void BBS2SpikeSim::checkbufleak() {
	if (BBS2SpikeSim::bufcnt_ > 0) {
		//printf("%d BBS2SpikeSim::bufcnt=%d\n", mpi_rank, bufcnt_);
	}
}
#endif


