// BBS2MPI.2.h
// Neuron's bbsmpipack file converted to MPI's c++ format 
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BBS2MPI_H
#define BBS2MPI_H

#include <cstdlib>
#include <iostream>
#include <algorithm>   // for copy
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

//Allow MPI
#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END
#include <mpi.h>



typedef struct bbsmpibuf {
    //std::vector<char> buf;
    std::string buf;
    int size;
    int pkposition;
    int upkpos;
    int keypos;
    int refcount;
} bbsmpibuf;

namespace BBS2MPI
{
//public:
//  BBS2MPI();
bbsmpibuf* newbuf(int size);
void free(bbsmpibuf* buf);
void copy(bbsmpibuf* dest, bbsmpibuf* src);
void ref(bbsmpibuf* buf);
void unref(bbsmpibuf* buf);
void pack(void* inbuf, int incount, int my_datatype, bbsmpibuf* r, const char* e);
void unpack(void* buf, int count, int my_datatype, bbsmpibuf* r, const char* errmes);
void upkbegin(bbsmpibuf* buf);
char* getkey(bbsmpibuf* buf);
int getid(bbsmpibuf* buf);
int upkint(bbsmpibuf* buf);
double upkdouble(bbsmpibuf* buf);
void upkvec(int n, double* x, bbsmpibuf* buf);
char* upkstr(bbsmpibuf* buf);
 void resize(bbsmpibuf* r, int size);
void pkbegin(bbsmpibuf* buf);
void enddata(bbsmpibuf* buf);
void pkint(int i, bbsmpibuf* buf);
void pkdouble(double x, bbsmpibuf* buf);
void pkvec(int n, double* x, bbsmpibuf* buf);
void pkstr(const char* s, bbsmpibuf* buf);

int iprobe(int* size, int* tag, int* source);
void bbssend(int dest, int tag, bbsmpibuf* r);
int bbsrecv(int source, bbsmpibuf* r);
int bbssendrecv(int dest, int tag, bbsmpibuf* s, bbsmpibuf* r);
}



#endif
