// DataRecordNeuron.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DATARECORDNEURON_H
#define DATARECORDNEURON_H


#include <deque>

#include "Visitor.h"
#include "Error.h"
#include "GlobalDefs.h"
#include "ConfigBase.h"
#include "Macros.h"
#include "InterfaceBase.h"
#include "DataCommonNeuron.h"
#include "SimulationEnvironment.h"


class DataRecordNeuron;
class DataRecordNeuronVisitor;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// DataRecordNeuronConfig class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

class DataRecordNeuronConfig
        : public DataCommonNeuronConfig
{
public:
    //! Type related to this configurator type.
    /*! Used for automated construction of neurons from configurators.
        See NeuronFactory and SynapseFactory.
     */
    typedef DataRecordNeuron related_component;

    //! Accept method for visitor (see class template Visitor).
    MAKE_VISITABLE(DataRecordNeuronConfig)
};



///////////////////////////////////////////////////////////////////////////////////////////////////
/// DataRecordNeuron definition
// for neurons that record their activity
// contains a list of the spike times
///////////////////////////////////////////////////////////////////////////////////////////////////

class DataRecordNeuron
        : public DataCommonNeuron
{
    friend class DataRecordNeuronVisitor;

// construction
protected:
    explicit DataRecordNeuron(ConfigBase * const configurator);
    void apply_vis_impl(AbstractVisitor & vis);

// update methods
protected:
    void notify_firing_impl(const Time & time_to_spike);

// members and accessors
protected:
    inline const std::deque<Time> & spike_time_list_impl() const {
        return spike_time_list_;
    }

private:
    std::deque<Time> spike_time_list_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
/// DataRecordNeuronVisitor class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

class DataRecordNeuronVisitor
        : public AbstractVisitor
        , public Visitor<DataRecordNeuron>
{
public:
    DataRecordNeuronVisitor() : spike_time_list_pointer_(0) {}
    void reset() {
        spike_time_list_pointer_ = 0;
    }
    void visit(DataRecordNeuron & drn) {
        spike_time_list_pointer_ = & drn.spike_time_list_;
    }
    std::deque<Time> * const spike_time_list_pointer() const {
        return spike_time_list_pointer_;
    }
    const std::deque<Time> & spike_time_list() const {
        return *spike_time_list_pointer_;
    }
    void erase_past_spike_list(const std::deque<Time>::iterator & it);
protected:
    std::deque<Time> * spike_time_list_pointer_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
//! constructor
inline DataRecordNeuron::DataRecordNeuron(ConfigBase * const configurator)
        : DataCommonNeuron(configurator)
{
    if (! dynamic_cast<DataRecordNeuronConfig *>(configurator))
        throw ConfigError("DataRecordNeuron: void configurator");
};

/////////////////////////////////////////////////
//! called when the neuron fires a spike (activation mechanism)
inline void DataRecordNeuron::notify_firing_impl(const Time & time_to_spike)
{
    // add the spike time to the list of spike times
    spike_time_list_.push_back(time_to_spike + SimEnv::sim_time());
    // notify the post synapses
    DataCommonNeuron::notify_firing_impl(time_to_spike);
}

/////////////////////////////////////////////////
//! to access the list of spike times
inline void DataRecordNeuron::apply_vis_impl(AbstractVisitor & vis)
{
    // check the compatibilty of the visitor and call the method visit if suitable
    DataRecordNeuronVisitor * ptvis = dynamic_cast<DataRecordNeuronVisitor *>(& vis);
    if (ptvis)
        ptvis->visit(*this);
    else
        // if the visitor is not suitable, send it to the base class
        DataCommonNeuron::apply_vis_impl(vis);
}


/////////////////////////////////////////////////
//! called to erase the past of the list of the spike times
// it is an iterator to indicate the position corresponding to the end of the "past" in the list
inline void DataRecordNeuronVisitor::erase_past_spike_list(const std::deque<Time>::iterator & it)
{
    // we have to use it2 and not it straight due to the const specification in const_iterator
    std::deque<Time>::iterator it2 = spike_time_list_pointer_->begin();
    it2 += (Size)(it - spike_time_list_pointer_->begin());
    spike_time_list_pointer_->erase(spike_time_list_pointer_->begin(), it2);
}


#endif // !defined(DATARECORDNEURON_H)
