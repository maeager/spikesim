// Threading.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "Threading.h"
#include "Network.h"
#include "ManageableInput.h"
#include "SimulationEnvironment.h"
#include "OutputManager.h"


pthread_t Threading::act_mech_threads[NUM_THREADS];
pthread_t Threading::plast_threads[NUM_THREADS];
pthread_mutex_t Threading::mutex_barrier_;
sem_t Threading::sem_grl_;
sem_t Threading::sem_[NUM_STEPS];
Size Threading::thread_count_[NUM_STEPS];

Threading::ListThreadObjType Threading::actmech_obj_;
Threading::ListThreadObjType Threading::plast_obj_;



void Threading::init(const Network & net)
{
    actmech_obj_.resize(NUM_THREADS);
    Size i = 0;
    for (ListThreadObjType::const_iterator it = net.threads_.begin();
            it != net.threads_.end();
            ++it, ++i)
        actmech_obj_[i] = * it;

    plast_obj_.resize(NUM_THREADS);
    i = 0;
    for (ListThreadObjType::iterator it = PlasticityManager::threads_.begin();
            it != PlasticityManager::threads_.end();
            ++it, ++i)
        plast_obj_[i] = * it;

    pthread_mutex_init(& mutex_barrier_, 0);

    pthread_mutex_lock(& mutex_barrier_);

    thread_count_[0] = 0;
    thread_count_[1] = 0;

    sem_init(& sem_grl_, 0, 0);
    for (Size i = 0; i < NUM_THREADS; ++i) sem_init(& sem_[i], 0, 0);

    pthread_mutex_unlock(& mutex_barrier_);

    for (Size i = 0; i < NUM_THREADS; ++i)
        pthread_create(& act_mech_threads[i], 0, threadable_object<ThreadableInterface, 0>, (void *)(actmech_obj_[i].operator->()));
    for (Size i = 0; i < NUM_THREADS; ++i)
        pthread_create(& plast_threads[i], 0, threadable_object<ThreadableInterface, 1>, (void *)(plast_obj_[i].operator->()));
}


void Threading::launch_sim(Network & net)
{
    while (SimEnv::i_time() < SimEnv::i_duration()) {
        ManageableInputManager::input_update_general();

        if ((SimEnv::i_time() % (OutputManager::i_outputting_period())) == 0) {
            // display the simulated time on the console
            std::cout << SimEnv::sim_time() << std::endl;
            // performs the recurrent outputting
            OutputManager::do_output("during_sim");
            // clean the past spike history of the record neurons
            OutputManager::clear_past_of_spike_lists(net);
        }

        pthread_mutex_lock(& mutex_barrier_);
        thread_count_[0] = NUM_THREADS;
        pthread_mutex_unlock(& mutex_barrier_);
        for (Size i = 0; i < NUM_THREADS; ++i)
            sem_post(& sem_[0]);
        sem_wait(& sem_grl_);

        pthread_mutex_lock(& mutex_barrier_);
        thread_count_[1] = NUM_THREADS;
        pthread_mutex_unlock(& mutex_barrier_);
        for (Size i = 0; i < NUM_THREADS; ++i)
            sem_post(& sem_[1]);
        sem_wait(& sem_grl_);

        SimEnv::advance();
//std::cout << std::endl << SimEnv::i_time() << "_"; std::cin.get();
    }

    for (Size i = 0; i < NUM_THREADS; ++i) {
        sem_post(& sem_[0]);
        sem_post(& sem_[1]);
    }
}


