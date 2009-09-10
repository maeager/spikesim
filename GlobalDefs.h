// GlobalDefs.h
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef GLOBALDEFS_H
#define GLOBALDEFS_H


#define NUM_THREADS 1

#if (NUM_THREADS > 1)
#define THREADED_ENGINE
#endif

//! Time in time steps (long unsigned).
typedef long unsigned DiscreteTime;

//! Time in seconds (double).
typedef double Time;

//! Size for arrays and lists (redefinition of unsigned integers).
typedef unsigned Size;

//! Neuron potential.
typedef double Volt;




#endif // !GLOBALDEFS_H
