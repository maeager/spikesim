// Threading.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NOTHREADING_H
#define NOTHREADING_H


#ifdef PARALLELSIM
class ParNetwork;
#else
class Network;
#endif

struct NoThreading {

#ifdef PARALLELSIM
    static void init(const ParNetwork & net) {}
    static void launch_sim(ParNetwork & net);
#else
    static void init(const Network & net) {}
    static void launch_sim(Network & net);
#endif

};




#endif // !defined(THREADING_H)
