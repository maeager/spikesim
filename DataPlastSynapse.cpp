// DataPlastSynapse.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "DataPlastSynapse.h"
#include "DataPlastNeuron.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// DataPlastSynapseConfig function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// constructor
DataPlastSynapseConfig::DataPlastSynapseConfig(DistributionManager * weightdistrib
        , DistributionManager * delaydistrib)
        : DataIndivSynapseConfig(weightdistrib, delaydistrib)
        , preneuron_spike_list_(0)
        , postneuron_spike_list_(0)
{
}

/////////////////////////////////////////////////
// gets pointers to the spike_time lists of the neurons
void DataPlastSynapseConfig::get_spike_list_from_pre_and_postneurons(NeuronInterface * const preneuron
        , NeuronInterface * const postneuron)
{
    // resets the pointers
    preneuron_spike_list_ = 0;
    postneuron_spike_list_ = 0;
    // retrieve the information about the list of the spike times for each of the pre- and post-neurons
    DataRecordNeuronVisitor vis;
    preneuron->apply_visitor(vis);
    preneuron_spike_list_ = vis.spike_time_list_pointer();
    // reset to ensure that the pointer 0 will be returned if there is a problem (unsuitable pre- or post-neuron type for instance)
    vis.reset();
    postneuron->apply_visitor(vis);
    postneuron_spike_list_ = vis.spike_time_list_pointer();
}
