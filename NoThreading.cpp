// NoThreading.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "NoThreading.h"
#ifdef PARALLELSIM
#include "ParNetwork.h"
#else
#include "Network.h"
#endif
#include "ManageableInput.h"
#include "DataPlastNeuron.h"
#include "SimulationEnvironment.h"
#include "OutputManager.h"

#ifdef PARALLELSIM
void NoThreading::launch_sim(ParNetwork & net)
#else
void NoThreading::launch_sim(Network & net)
#endif
{
    while (SimEnv::i_time() < SimEnv::i_duration()) {
        // input updates
        ManageableInputManager::input_update_general();

        // output recordings to files and cleaning of the spike_lists
        if ((SimEnv::i_time() % (OutputManager::i_outputting_period())) == 0) {
            // display the simulated time on the console
            std::cout << SimEnv::sim_time() << std::endl;
            // performs the recurrent outputting
            OutputManager::do_output("during_sim");
            // clean the past spike history of the record neurons
            OutputManager::clear_past_of_spike_lists(net);
        }

        // performs the outputting at each time step
        OutputManager::do_output("each_time_step");

        // activation update of all the neurons (they call the update of the synapses)
        net.update();

        // weight updates of the concerned plastic synapses (with the class DataPlastNeuron)
        if (SimEnv::sim_time() >= SimEnv::plasticity_effective_start_time())
            PlasticityManager::plast_update_general(); // only updates plastic neurons

        // advance the simulated time a timestep further
        SimEnv::advance();
    }
}

