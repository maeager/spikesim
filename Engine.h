// Engine.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ENGINE_H
#define ENGINE_H

#ifdef PARALLELSIM
class ParNetwork;
#else
class Network;
#endif

template <class ThreadingType>
struct Engine {
#ifdef PARALLELSIM
    void init(const ParNetwork & net) {
        ThreadingType::init(net);
    }
    void launch_sim(ParNetwork & net) {
        ThreadingType::launch_sim(net);
    }

#else
    void init(const Network & net) {
        ThreadingType::init(net);
    }
    void launch_sim(Network & net) {
        ThreadingType::launch_sim(net);
    }
#endif
};


#endif // !defined(ENGINE_H)
