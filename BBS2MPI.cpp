
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "BBS2MPI.h"
#include "ParSpike.h"
#include <mpi.h>
#include <errno.h>
#define debugleak 0
//#define DEBUG 0

extern MPI_Comm bbs_comm;

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

static MPI_Datatype mytypes[] = {MPI_INT, MPI_DOUBLE, MPI_CHAR, MPI_PACKED};


/*BBS2MPI::BBS2MPI()
{

}*/

 /** 
  * Unpack method
  * 
  * @param buf 
  * @param count 
  * @param my_datatype 
  * @param r 
  * @param errmes 
  */
void BBS2MPI::unpack(void* buf, int count, int my_datatype, bbsmpibuf* r, const char* errmes)
{
    int type[2];
    assert(r);
#if DEBUG == 2 
    std::cout << ParSpike::my_rank<< " unpack upkpos=" << r->upkpos<< " pkposition=" << r->pkposition<< " keypos="<< r->keypos<<" size="<< r->size<< std::endl;
#endif
    assert(r->upkpos >= 0 && r->size >= r->upkpos);
    MPI_Unpack(&(r->buf[0]), r->size, &r->upkpos, type, 2, MPI_INT, bbs_comm);
#if DEBUG == 2
std::cout<<ParSpike::my_rank<<" unpack r="<< (long)r<<" size="<< r->size<<" upkpos="<< r->upkpos<<" type[0]="<< type[0]<<" datatype="<< my_datatype<<"  type[1]="<< type[1]<<"  count="<< count<<std::endl;
#endif
    if (type[0] != my_datatype || type[1] != count) {
std::cout<<ParSpike::my_rank<<" unpack size="<<r->size<<" upkpos="<<r->upkpos<<" type[0]="<<type[0]<<"   datatype="<<my_datatype<<"  type[1]="<< type[1]<<"  count="<<count<<std::cout;
    }
    assert(type[0] == my_datatype);
    assert(type[1] == count);
    MPI_Unpack(&(r->buf[0]), r->size, &r->upkpos, buf, count, mytypes[my_datatype], bbs_comm);
}
/** 
 * 
 * 
 * @param r 
 */
void BBS2MPI::upkbegin(bbsmpibuf* r)
{
    int type;
    int p;
#if DEBUG == 2
std::cout<< ParSpike::my_rank<<" BBS2MPI::upkbegin "<<(long)r<<" (preunpack upkpos="<< r->upkpos<<" keypos="<< r->keypos<< std::endl;
#endif
    assert(r  && r->size > 0);
    r->upkpos = 0;
    MPI_Unpack(&r->buf[0], r->size, &r->upkpos,
               &p, 1, MPI_INT, bbs_comm);
    if (p > r->size) {
std::cout <<  ParSpike::my_rank<< "  BBS2MPI::upkbegin keypos=" <<  p << " size=" <<  r->size<< std::endl;
    }
    assert(p <= r->size);
    MPI_Unpack(&(r->buf[0]), r->size, &p, &type, 1, MPI_INT, bbs_comm);
#if DEBUG == 2
 std::cout <<  ParSpike::my_rank << "BBS2MPI::upkbegin type=" <<  type << " keypos=" <<  p<< std::endl;
#endif
    assert(type == 0);
    r->keypos = p;
}
/** 
 * Get the key of new buffer
 * 
 * @param r 
 * 
 * @return 
 */
char* BBS2MPI::getkey(bbsmpibuf* r)
{
    char* s;
    int type;
    type = r->upkpos;
    r->upkpos = r->keypos;
#if DEBUG == 2
 std::cout <<  ParSpike::my_rank << " BBS2MPI::getkey " <<  (long)r << " keypos=" <<  r->keypos  << std::endl;
#endif
    s = BBS2MPI::upkstr(r);
    assert(r->pkposition == 0 || r->pkposition == r->upkpos);
    r->pkposition = r->upkpos;
    r->upkpos = type;
#if DEBUG == 2
std::cout<< " getkey return  " <<  s<< std::endl;
#endif
    return s;
}

/** 
 * Get the ID in buffer
 * 
 * @param r buffer
 * 
 * @return int of buffer
 */
int BBS2MPI::getid(bbsmpibuf* r)
{
    int i, type;
    type = r->upkpos;
    r->upkpos = r->keypos;
#if DEBUG == 2
 std::cout <<  ParSpike::my_rank << " BBS2MPI::getid " <<  (long)r << " keypos=" <<  r->keypos  << std::endl;
#endif
    i = BBS2MPI::upkint(r);
    r->upkpos = type;
#if DEBUG == 2
std::cout<< " getid return  " <<  i<< std::endl;
#endif
    return i;
}
/** 
 * Unpack an integer
 * 
 * @param r 
 * 
 * @return 
 */
int BBS2MPI::upkint(bbsmpibuf* r)
{
    int i;
    unpack(&i, 1, my_MPI_INT, r, "upkint");
    return i;
}
/** 
 * Unpack a double
 * 
 * @param r 
 * 
 * @return 
 */
double BBS2MPI::upkdouble(bbsmpibuf* r)
{
    double x;
    unpack(&x, 1, my_MPI_DOUBLE, r, "upkdouble");
    return x;
}
/** 
 * Unpack an array of doubles
 * 
 * @param n 
 * @param x 
 * @param r 
 */
void BBS2MPI::upkvec(int n, double* x, bbsmpibuf* r)
{
    unpack(x, n, my_MPI_DOUBLE, r, "upkvec");
}
/** 
 * Unpack string
 * 
 * @param r 
 * 
 * @return 
 */
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

static void resize(bbsmpibuf* r, int size)
{
    int newsize;
    if (r->size < size) {
        newsize = (size / 64) * 64 + 128;
        r->buf.resize(newsize);
        r->size = newsize;
    }
}
/** 
 * begin packing
 * 
 * @param r 
 */
void BBS2MPI::pkbegin(bbsmpibuf* r)
{
    int type;
    r->pkposition = 0;
    type = 0;
#if DEBUG == 2
 std::cout <<  ParSpike::my_rank << " BBS2MPI::pkbegin " <<  (long)r << " size=" <<  r->size << " pkposition=" <<  r->pkposition << std::endl;
#endif
    MPI_Pack(&type, 1, MPI_INT, &(r->buf[0]), r->size, &r->pkposition, bbs_comm);
}
/** 
 * 
 * 
 * @param r 
 */
void BBS2MPI::enddata(bbsmpibuf* r)
{
    int p, type, isize, oldsize;
    p = r->pkposition;
    type = 0;
#if DEBUG == 2
 std::cout <<  ParSpike::my_rank << " BBS2MPI::enddata " <<  (long)r << " size=" <<  r->size << " pkposition=" <<  p << std::endl;
#endif
    MPI_Pack_size(1, MPI_INT, bbs_comm, &isize);
    oldsize = r->size;
    resize(r, r->pkposition + isize);
#if DEBUG == 2
    if (oldsize < r->pkposition + isize) {
         std::cout <<  ParSpike::my_rank << " " <<  (long)r << " need " <<  isize << " more. end up with total of " <<  r->size << std::endl;
    }
#endif
    MPI_Pack(&type, 1, MPI_INT, &(r->buf[0]), r->size, &r->pkposition, bbs_comm);
#if DEBUG == 2
 std::cout <<  ParSpike::my_rank << " BBS2MPI::enddata buf=";
for(register int i=0; i<r->buf.size(); i++) std::cout << r->buf[i];
 std::cout<< " size=" <<  r->size << " pkposition=" <<  r->pkposition << std::endl;
#endif
    MPI_Pack(&p, 1, MPI_INT, &(r->buf[0]), r->size, &type, bbs_comm);
#if DEBUG == 2
 std::cout <<  ParSpike::my_rank << "after BBS2MPI::enddata, " <<  p << " was packed at beginning and 0 was packed before " <<  r->pkposition<< std::endl;
#endif
}

/** 
 * Pack buffer generic method
 * 
 * @param inbuf 
 * @param incount 
 * @param my_datatype 
 * @param r buffer
 * @param e error string
 */
void BBS2MPI::pack(void* inbuf, int incount, int my_datatype, bbsmpibuf* r, const char* e)
{
    int type[2];
    int dsize, isize, oldsize;
#if DEBUG == 2
std::cout<< ParSpike::my_rank<<" pack "<< (long)r<< " count="<<incount<<" type="<<my_datatype<<" outbuf-";
for (register int i=0; i< r->buf.size();i++) std::cout<< r->buf[i];
std::cout<<" pkposition=" << r->pkposition << " " << e << std::endl;
#endif
    MPI_Pack_size(incount, mytypes[my_datatype], bbs_comm, &dsize);
    MPI_Pack_size(2, MPI_INT, bbs_comm, &isize);
    oldsize = r->size;
    resize(r, r->pkposition + dsize + isize);
#if DEBUG == 2
    if (oldsize < r->pkposition + dsize + isize) {
         std::cout <<  ParSpike::my_rank << " " <<  (long)r << " need " <<  dsize+isize << " more. end up with total of " <<  r->size << std::endl;
    }
#endif
    type[0] = my_datatype;  type[1] = incount;
    MPI_Pack(type, 2, MPI_INT, &(r->buf[0]), r->size, &r->pkposition, bbs_comm);
    MPI_Pack(inbuf, incount, mytypes[my_datatype], &(r->buf[0]), r->size, &r->pkposition, bbs_comm);
#if DEBUG == 2
 std::cout <<  ParSpike::my_rank << " pack done pkposition=" <<  r->pkposition << std::endl;
#endif
}

/** 
 * Pack integer
 * 
 * @param i 
 * @param r 
 */
void BBS2MPI::pkint(int i, bbsmpibuf* r)
{
    int ii;
    ii = i;
    pack(&ii, 1, my_MPI_INT, r, "pkint");
}
/** 
 * 
 * 
 * @param x 
 * @param r 
 */
void BBS2MPI::pkdouble(double x, bbsmpibuf* r)
{
    double xx;
    xx = x;
    pack(&xx, 1, my_MPI_DOUBLE, r, "pkdouble");
}
/** 
 * Pack vector of doubles
 * 
 * @param n size of array
 * @param x array pointer
 * @param r buffer
 */
void BBS2MPI::pkvec(int n, double* x, bbsmpibuf* r)
{
    pack(x, n, my_MPI_DOUBLE, r, "pkvec");
}
/** 
 * 
 * 
 * @param s string
 * @param r buffer
 */
void BBS2MPI::pkstr(const char* s, bbsmpibuf* r)
{
    int len;
    len = strlen(s);
    pack(&len, 1, my_MPI_INT, r, "pkstr length");
    pack((char*)s, len, my_MPI_CHAR, r, "pkstr string");
}
/** 
 * Send the buffer
 * 
 * @param dest target node
 * @param tag 
 * @param r 
 */
void BBS2MPI::bbssend(int dest, int tag, bbsmpibuf* r)
{
#if DEBUG == 2
std::cout <<  ParSpike::my_rank << " BBS2MPI::bbssend " <<  (long)r << " dest=" <<  dest << " tag=" <<  tag << " size=" <<  (int)((r)?r->upkpos:0)  << std::endl;
#endif
    if (r) {
        assert(r->keypos <= r->size);
        MPI_Send(&(r->buf[0]), r->pkposition, MPI_PACKED, dest, tag, bbs_comm);
    } else {
        MPI_Send(NULL, 0, MPI_PACKED, dest, tag, bbs_comm);
    }
    errno = 0;
#if DEBUG == 2
 std::cout <<  ParSpike::my_rank << "return from send" << std::endl;
#endif
}
/** 
 * Receive a buffer
 * 
 * @param source 
 * @param r 
 * 
 * @return 
 */
int BBS2MPI::bbsrecv(int source, bbsmpibuf* r)
{
    MPI_Status status;
    int size;
    if (source == -1) {
        source = MPI_ANY_SOURCE;
    }
#if DEBUG == 2
 std::cout <<  ParSpike::my_rank << " BBS2MPI::bbsrecv " << std::endl;
#endif
    MPI_Probe(source, MPI_ANY_TAG, bbs_comm, &status);
    MPI_Get_count(&status, MPI_PACKED, &size);
#if DEBUG == 2
 std::cout <<  ParSpike::my_rank << "BBS2MPI::bbsrecv probe size=" <<  size << "source=" <<  status.MPI_SOURCE << "tag=" <<  status.MPI_TAG << std::endl;
#endif
    resize(r, size);
    MPI_Recv(&(r->buf[0]), r->size, MPI_PACKED, source, MPI_ANY_TAG, bbs_comm, &status);
    errno = 0;
    return status.MPI_TAG;
}
/** 
 * Send and Recv a buffer
 * 
 * @param dest 
 * @param tag 
 * @param s 
 * @param r 
 * 
 * @return 
 */
int BBS2MPI::bbssendrecv(int dest, int tag, bbsmpibuf* s, bbsmpibuf* r)
{
    int size, itag, source;
    int msgtag;
    MPI_Status status;
#if DEBUG == 2
 std::cout <<  ParSpike::my_rank << "BBS2MPI::bbssendrecv dest=" <<  dest << " tag=" <<  tag<< std::endl;
#endif
    if (!BBS2MPI::iprobe(&size, &itag, &source) || source != dest) {
#if DEBUG == 2
 std::cout <<  ParSpike::my_rank << "BBS2MPI::bbssendrecv nothing available so send" << std::endl;
#endif
        BBS2MPI::bbssend(dest, tag, s);
    }
    return BBS2MPI::bbsrecv(dest, r);
}
/** 
 * Probe sources
 * 
 * @param size 
 * @param tag 
 * @param source 
 * 
 * @return 
 */
int BBS2MPI::iprobe(int* size, int* tag, int* source)
{
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
/** 
 * Create a new buffer
 * 
 * @param size 
 * 
 * @return 
 */
bbsmpibuf* BBS2MPI::newbuf(int size)
{

    bbsmpibuf* buf = new bbsmpibuf;
#if DEBUG == 2
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
/** 
 * Copy and existing buffer
 * 
 * @param dest 
 * @param src 
 */
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
/** 
 * Clean up buffers
 * 
 * @param buf 
 */
void BBS2MPI::free(bbsmpibuf* buf)
{
#if DEBUG == 2
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
/** 
 * Increment references to buffer
 * 
 * @param buf 
 */
void BBS2MPI::ref(bbsmpibuf* buf)
{
    assert(buf);
    buf->refcount += 1;
}
/** 
 * Decrement references to buffer
 * 
 * @param buf 
 */
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


