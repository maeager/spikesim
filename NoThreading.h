// Threading.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NOTHREADING_H
#define NOTHREADING_H

class Network;

struct NoThreading
{
	static void init(const Network & net) {}
	static void launch_sim(Network & net);
};




#endif // !defined(THREADING_H)
