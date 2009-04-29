// Threading.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef THREADING_H
#define THREADING_H

#include <iostream>

#include <pthread.h>
#include <semaphore.h> 
#include <vector>
#include <boost/shared_ptr.hpp>

#include "GlobalDefs.h"
#include "SimulationEnvironment.h"


#define NUM_STEPS 2

#pragma warning( disable : 4311 )
#pragma warning( disable : 4312 )


class ThreadableInterface
{
    friend class Network;
    friend class PlasticityManager;
public:
	virtual ~ThreadableInterface() {}
	template <int Num> void launch();
protected:
	virtual void operation() = 0;
};


template <class Type, int Num>
void * threadable_object(void * arg)
{
    (static_cast<Type *>(arg))->template launch<Num>();
	pthread_exit(0);
	return 0;
}


class Network;

class Threading
{
	friend class ThreadableInterface;
public:
	static void init(const Network & net);
	static void launch_sim(Network & net);
	
private:
	static void lock() {pthread_mutex_lock(& Threading::mutex_barrier_);}
	static void unlock() {pthread_mutex_unlock(& Threading::mutex_barrier_);}
	static void post_grl() {sem_post(& Threading::sem_grl_);}

    static pthread_mutex_t mutex_barrier_;
	static pthread_t act_mech_threads[NUM_THREADS];
	static pthread_t plast_threads[NUM_THREADS];
	static sem_t sem_grl_;
	static sem_t sem_[NUM_STEPS];
	static Size thread_count_[NUM_STEPS];

    typedef std::vector<boost::shared_ptr<ThreadableInterface> > ListThreadObjType;
	static ListThreadObjType actmech_obj_;
	static ListThreadObjType plast_obj_;
};


template <int Num>
void ThreadableInterface::launch()
{
	sem_wait(& Threading::sem_[Num]);

	while (SimEnv::i_time() < SimEnv::i_duration())
	{
        operation();
		
		Threading::lock();
		--Threading::thread_count_[Num];
		if (Threading::thread_count_[Num] == 0) Threading::post_grl();
		Threading::unlock();

		sem_wait(& Threading::sem_[Num]);
	}
}



#endif // !defined(THREADING_H)
