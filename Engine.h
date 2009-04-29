// Engine.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ENGINE_H
#define ENGINE_H


class Network;

template <class ThreadingType>
struct Engine
{
	void init(const Network & net) {ThreadingType::init(net);}
	void launch_sim(Network & net) {ThreadingType::launch_sim(net);}
};


#endif // !defined(ENGINE_H)
