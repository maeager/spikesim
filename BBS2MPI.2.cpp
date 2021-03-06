
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <mpi.h>
#include <errno.h>
#define debugleak 0
#define debug 0

#include "BBS2MPI.2.h"
#include "ParSpike.2.h"

extern MPI::Intracomm bbs_comm;

#if debugleak
static int BBS2MPI::bufcnt_;
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

//static MPI_Datatype mytypes[] = {MPI_INT, MPI_DOUBLE, MPI_CHAR, MPI_PACKED};
static MPI::Datatype mytypes[] = {MPI_INT, MPI_DOUBLE, MPI_CHAR, MPI_PACKED};


/*BBS2MPI::BBS2MPI()
{

}*/

void BBS2MPI::unpack(void* buf, int count, int my_datatype, bbsmpibuf* r, const char* errmes)
{
    int type[2];
    assert(r);
#if DEBUG
 std::cout << ParSpike::my_rank << " unpack upkpos=" <<  r->upkpos << " pkposition=" <<  r->pkposition << " keypos=" <<  r->keypos << " size=" <<  r->size << std::endl;
#endif
    assert(r->upkpos >= 0 && r->size >= r->upkpos);
    MPI::INT.Unpack((const void*)&(r->buf[0]), r->size, (void*)type, 2, (int&)r->upkpos, (const MPI::Comm&)bbs_comm);
#if DEBUG
std::cout << ParSpike::my_rank << "unpack r=" << (long)r<<" size="<< r->size<<" upkpos="<<  r->upkpos<<" type[0]="<<type[0] <<" datatype="<<my_datatype <<"  type[1]="<< type[1]<<"  count="<<  count<< std::endl;
#endif
    if (type[0] != my_datatype || type[1] != count) {
std::cout<< ParSpike::my_rank<<" unpack size="<< r->size<<" upkpos="<<r->upkpos <<" type[0]="<<type[0] <<"   datatype="<< my_datatype<<"  type[1]="<< type[1] <<"  count="<< count<<std::endl;

    }
    assert(type[0] == my_datatype);
    assert(type[1] == count);
    mytypes[my_datatype].Unpack((const void*)&(r->buf[0]), (int) r->size, (void*)buf, count, (int&)r->upkpos, (const MPI::Comm&)bbs_comm);
}

void BBS2MPI::upkbegin(bbsmpibuf* r)
{
    int type;
    int p;
#if DEBUG
std::cout<< ParSpike::my_rank<<" BBS2MPI::upkbegin "<< (long)r<<" (preunpack upkpos="<<r->upkpos <<" keypos="<< r->keypos<<")"<<std::endl;
#endif
    assert(r  && r->size > 0);
    r->upkpos = 0;
    MPI::INT.Unpack((const void*)&r->buf[0], (int)r->size, (void*)&p, 1, (int&)r->upkpos, (const MPI::Comm&)bbs_comm);
    if (p > r->size) {
std::cout<<ParSpike::my_rank<<" BBS2MPI::upkbegin keypos="<< p<< " size="<<r->size<<std::endl;
    }
    assert(p <= r->size);
    MPI::INT.Unpack((const void*)&(r->buf[0]), (int) r->size, (void*)&type, 1, p, (const MPI::Comm&)bbs_comm);
#if DEBUG
 std::cout <<  ParSpike::my_rank << "BBS2MPI::upkbegin type=" <<  type << " keypos=" <<  p<< std::endl;
#endif
    assert(type == 0);
    r->keypos = p;
}

char* BBS2MPI::getkey(bbsmpibuf* r)
{
    char* s;
    int type;
    type = r->upkpos;
    r->upkpos = r->keypos;
#if DEBUG
 std::cout <<  ParSpike::my_rank << " BBS2MPI::getkey " <<  (long)r << " keypos=" <<  r->keypos  << std::endl;
#endif
    s = BBS2MPI::upkstr(r);
    assert(r->pkposition == 0 || r->pkposition == r->upkpos);
    r->pkposition = r->upkpos;
    r->upkpos = type;
#if DEBUG
std::cout << "getkey return "<< s<< std::endl;
#endif
    return s;
}

int BBS2MPI::getid(bbsmpibuf* r)
{
    int i, type;
    type = r->upkpos;
    r->upkpos = r->keypos;
#if DEBUG
 std::cout <<  ParSpike::my_rank << " BBS2MPI::getid " <<  (long)r << " keypos=" <<  r->keypos  << std::endl;
#endif
    i = BBS2MPI::upkint(r);
    r->upkpos = type;
#if DEBUG
std::cout<< " getid return  " <<  i<< std::endl;
#endif
    return i;
}

int BBS2MPI::upkint(bbsmpibuf* r)
{
    int i;
    unpack(&i, 1, my_MPI_INT, r, "upkint");
    return i;
}

double BBS2MPI::upkdouble(bbsmpibuf* r)
{
    double x;
    unpack(&x, 1, my_MPI_DOUBLE, r, "upkdouble");
    return x;
}

void BBS2MPI::upkvec(int n, double* x, bbsmpibuf* r)
{
    unpack(x, n, my_MPI_DOUBLE, r, "upkvec");
}

char* BBS2MPI::upkstr(bbsmpibuf* r)
{
    int len;
    char* s;
    unpack(&len, 1, my_MPI_INT, r, "upkstr length");
    s = new char[len+1];
    unpack(s, len, my_MPI_CHAR, r, "upkstr string");
    s[len] = '\0';
    return s;
}

void BBS2MPI::resize(bbsmpibuf* r, int size)
{
    int newsize;
    if (r->size < size) {
        newsize = (size / 64) * 64 + 128;
        r->buf.resize(newsize);
        r->size = newsize;
    }
}

void BBS2MPI::pkbegin(bbsmpibuf* r)
{
    int type;
    r->pkposition = 0;
    type = 0;
#if DEBUG
 std::cout <<  ParSpike::my_rank << " BBS2MPI::pkbegin " <<  (long)r << " size=" <<  r->size << " pkposition=" <<  r->pkposition << std::endl;
#endif
    MPI::INT.Pack((const void*)&type, 1, (void*) &(r->buf[0]), r->size, r->pkposition, bbs_comm);
}

void BBS2MPI::enddata(bbsmpibuf* r)
{
    int p, type, isize, oldsize;
    p = r->pkposition;
    type = 0;
#if DEBUG
 std::cout <<  ParSpike::my_rank << " BBS2MPI::enddata " <<  (long)r << " size=" <<  r->size << " pkposition=" <<  p << std::endl;
#endif
    isize = MPI::INT.Pack_size(1, bbs_comm);
    oldsize = r->size;
    resize(r, r->pkposition + isize);
#if DEBUG
    if (oldsize < r->pkposition + isize) {
         std::cout <<  ParSpike::my_rank << " " <<  (long)r << " need " <<  isize << " more. end up with total of " <<  r->size << std::endl;
    }
#endif
    MPI::INT.Pack((const void*)&type, 1, (void*)&(r->buf[0]), r->size, r->pkposition, bbs_comm);
#if DEBUG
 std::cout <<  ParSpike::my_rank << " BBS2MPI::enddata buf=" <<  r->buf << " size=" <<  r->size << " pkposition=" <<  r->pkposition << std::endl;
#endif
    MPI::INT.Pack((const void*)&p, 1, (void*)&(r->buf[0]), r->size, type, bbs_comm);
#if DEBUG
 std::cout <<  ParSpike::my_rank << "after BBS2MPI::enddata, " <<  p << " was packed at beginning and 0 was packed before " <<  r->pkposition<< std::endl;
#endif
}

void BBS2MPI::pack(void* inbuf, int incount, int my_datatype, bbsmpibuf* r, const char* e)
{
    int type[2];
    int dsize, isize, oldsize;
#if DEBUG
std::cout << ParSpike::my_rank<<" pack "<< (long)r<<" count="<< incount<<" type="<< my_datatype<<" outbuf-"<< r->buf<<" pkposition="<< r->pkposition <<" "<< e<<std::endl;

#endif
    dsize = mytypes[my_datatype].Pack_size(incount, bbs_comm);
    isize = MPI::INT.Pack_size(2, bbs_comm);
    oldsize = r->size;
    resize(r, r->pkposition + dsize + isize);
#if DEBUG
    if (oldsize < r->pkposition + dsize + isize) {
         std::cout <<  ParSpike::my_rank << " " <<  (long)r << " need " <<  dsize+isize << " more. end up with total of " <<  r->size << std::endl;
    }
#endif
    type[0] = my_datatype;  type[1] = incount;
    MPI::INT.Pack((const void*)type, 2, (void*)&(r->buf[0]), r->size, r->pkposition, bbs_comm);
    mytypes[my_datatype].Pack((const void*) inbuf,
                              incount,
                              (void*)&(r->buf[0]),
                              r->size,
                              r->pkposition,
                              bbs_comm);
#if DEBUG
 std::cout <<  ParSpike::my_rank << " pack done pkposition=" <<  r->pkposition << std::endl;
#endif
}

void BBS2MPI::pkint(int i, bbsmpibuf* r)
{
    int ii;
    ii = i;
    pack(&ii, 1, my_MPI_INT, r, "pkint");
}

void BBS2MPI::pkdouble(double x, bbsmpibuf* r)
{
    double xx;
    xx = x;
    pack(&xx, 1, my_MPI_DOUBLE, r, "pkdouble");
}

void BBS2MPI::pkvec(int n, double* x, bbsmpibuf* r)
{
    pack(x, n, my_MPI_DOUBLE, r, "pkvec");
}

void BBS2MPI::pkstr(const char* s, bbsmpibuf* r)
{
    int len;
    len = strlen(s);
    pack(&len, 1, my_MPI_INT, r, "pkstr length");
    pack((char*)s, len, my_MPI_CHAR, r, "pkstr string");
}

void BBS2MPI::bbssend(int dest, int tag, bbsmpibuf* r)
{
#if DEBUG
std::cout <<  ParSpike::my_rank << " BBS2MPI::bbssend  dest=" <<  dest << " tag=" <<  tag << " size= X"  << std::endl;
#endif
    if (r) {
        assert(r->keypos <= r->size);
        bbs_comm.Send(&(r->buf[0]), r->pkposition, MPI::PACKED, dest, tag);
    } else {
        bbs_comm.Send(NULL, 0, MPI::PACKED, dest, tag);
    }
    errno = 0;
#if DEBUG
 std::cout <<  ParSpike::my_rank << "return from send" << std::endl;
#endif
}

int BBS2MPI::bbsrecv(int source, bbsmpibuf* r)
{
    MPI::Status status;
    int size;
    if (source == -1) {
        source = MPI_ANY_SOURCE;
    }
#if DEBUG
 std::cout <<  ParSpike::my_rank << " BBS2MPI::bbsrecv " <<  (long)r << std::endl;
#endif
    bbs_comm.Probe(source, MPI_ANY_TAG, status);
    size = status.Get_count(MPI_PACKED);
#if DEBUG
 std::cout <<  ParSpike::my_rank << "BBS2MPI::bbsrecv probe size=" <<  size << "source=" <<  status.Get_source() << "tag=" <<  status.Get_tag() << std::endl;
#endif
    resize(r, size);
    bbs_comm.Recv(&(r->buf[0]), r->size, MPI_PACKED, source, MPI_ANY_TAG, status);
    errno = 0;
    return status.Get_tag();
}

int BBS2MPI::bbssendrecv(int dest, int tag, bbsmpibuf* s, bbsmpibuf* r)
{
    int size, itag, source;
    int msgtag;
    MPI::Status status;
#if DEBUG
 std::cout <<  ParSpike::my_rank << "BBS2MPI::bbssendrecv dest=" <<  dest << " tag=" <<  tag<< std::endl;
#endif
    if (!BBS2MPI::iprobe(&size, &itag, &source) || source != dest) {
#if DEBUG
 std::cout <<  ParSpike::my_rank << "BBS2MPI::bbssendrecv nothing available so send" << std::endl;
#endif
        BBS2MPI::bbssend(dest, tag, s);
    }
    return BBS2MPI::bbsrecv(dest, r);
}

int BBS2MPI::iprobe(int* size, int* tag, int* source)
{
    int  flag = 0;
    MPI::Status status;
    flag = (int) bbs_comm.Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, status);
    if (flag) {
        if (source) *source = status.Get_source();
        if (tag) *tag = status.Get_tag();
        if (size) *size = status.Get_count(MPI_PACKED);
    }
    return flag;
}

bbsmpibuf* BBS2MPI::newbuf(int size)
{

    bbsmpibuf* buf = new bbsmpibuf;
#if DEBUG
 std::cout <<  ParSpike::my_rank << " BBS2MPI::newbuf " <<  (long)buf << std::endl;
#endif
//  buf->buf = (char*)0;
    buf->buf.resize(size);
    buf->size = size;
    buf->pkposition = 0;
    buf->upkpos = 0;
    buf->keypos = 0;
    buf->refcount = 0;
#if debugleak
    ++BBS2MPI::bufcnt_;
#endif
    return buf;
}

void BBS2MPI::copy(bbsmpibuf* dest, bbsmpibuf* src)
{
    int i;
    resize(dest, src->size);
//  for (i=0; i < src->size; ++i) {
//      dest->buf[i] = src->buf[i];
//  }
    std::copy(src->buf.begin(), src->buf.end(), dest->buf.begin());
    dest->pkposition = src->pkposition;
    dest->upkpos = src->upkpos;
    dest->keypos = src->keypos;
}

void BBS2MPI::free(bbsmpibuf* buf)
{
#if DEBUG
 std::cout <<  ParSpike::my_rank << " BBS2MPI::free " <<  (long)buf << std::endl;
#endif
//  if (buf->buf) {
//      free(buf->buf);     /* STL vector automatic deletion */
//  }
//  free(buf);              /* boost::shared_ptr auto deletion */
    delete buf;
#if debugleak
    --BBS2MPI::bufcnt_;
#endif
}

void BBS2MPI::ref(bbsmpibuf* buf)
{
    assert(buf);
    buf->refcount += 1;
}

void BBS2MPI::unref(bbsmpibuf* buf)
{
    if (buf) {
        --buf->refcount;
        if (buf->refcount <= 0) {
            BBS2MPI::free(buf);
        }
    }
}

#if debugleak
void BBS2MPI::checkbufleak()
{
    if (BBS2MPI::bufcnt_ > 0) {
         std::cout <<  ParSpike::my_rank << " BBS2MPI::bufcnt=" <<  bufcnt_ << std::endl;
    }
}
#endif


