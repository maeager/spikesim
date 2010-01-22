// DataPlastSynapse.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DATAPLASTSYNAPSE_H
#define DATAPLASTSYNAPSE_H

#include <deque>

#include "GlobalDefs.h"
#include "ConfigBase.h"
#include "InterfaceBase.h"
#include "DataIndivSynapse.h"



class DataPlastSynapse;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// DataPlastSynapseConfig class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

class DataPlastSynapseConfig
        : public DataIndivSynapseConfig
{
public:
    //! Type related to this configurator type.
    /*! Used for automated construction of neurons from configurators.
        See NeuronFactory and SynapseFactory.
     */
    typedef DataPlastSynapse related_component;

    //! Accept method for visitor (see class template Visitor).
    MAKE_VISITABLE(DataPlastSynapseConfig)

public:
    DataPlastSynapseConfig(DistributionManager * weightdistrib, DistributionManager * delaydistrib);
    void get_spike_list_from_pre_and_postneurons(NeuronInterface * const preneuron, NeuronInterface * const postneuron);
    std::deque<Time> * const preneuron_spike_list() const {
        return preneuron_spike_list_;
    }
    std::deque<Time> * const postneuron_spike_list() const {
        return postneuron_spike_list_;
    }
private:
    std::deque<Time> * preneuron_spike_list_
    , * postneuron_spike_list_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
/// DataPlastSynapse definition
// for synapses that share same weights and delays
// contains a reference to the data
///////////////////////////////////////////////////////////////////////////////////////////////////

class DataPlastSynapse
        : public DataIndivSynapse
{
// construction
protected:
    explicit DataPlastSynapse(ConfigBase * const configurator);

// members and accessors
protected:
    inline const std::deque<Time> & preneuron_spike_list_impl() const {
        return *preneuron_spike_list_;
    }
    inline const std::deque<Time> & postneuron_spike_list_impl() const {
        return *postneuron_spike_list_;
    }
private:
    std::deque<Time> * const preneuron_spike_list_ // pointer to the spike_list of the preneuron
    , * const postneuron_spike_list_; // pointer to the spike_list of the postneuron
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// config constructor
inline DataPlastSynapse::DataPlastSynapse(ConfigBase * const configurator)
        : DataIndivSynapse(configurator)
        , preneuron_spike_list_(dynamic_cast<DataPlastSynapseConfig *>(configurator)->preneuron_spike_list())
        , postneuron_spike_list_(dynamic_cast<DataPlastSynapseConfig *>(configurator)->postneuron_spike_list())
{
    if (!(preneuron_spike_list_ && postneuron_spike_list_)) throw ConfigError("DataPlastSynapse: pre- or post-neuronspike_list_ not initialised");
}



#endif // !defined(DATAPLASTSYNAPSE_H)
