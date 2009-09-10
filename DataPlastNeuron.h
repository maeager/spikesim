// DataPlastNeuron.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DATAPLASTNEURON_H
#define DATAPLASTNEURON_H


#include <list>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "GlobalDefs.h"
#include "SimulationEnvironment.h"
#include "ConfigBase.h"
#include "DataRecordNeuron.h"
#include "InterfaceBase.h"
#include "Macros.h"



class DataPlastNeuron;


///////////////////////////////////////////////////////////////////////////////////////////////////
// DataPlastNeuronConfig class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

class DataPlastNeuronConfig
        : public DataRecordNeuronConfig
{
public:
    //! Type related to this configurator type.
    /*! Used for automated construction of neurons from configurators.
        See NeuronFactory and SynapseFactory.
     */
    typedef DataPlastNeuron related_component;

    //! Accept method for visitor (see class template Visitor).
    MAKE_VISITABLE(DataPlastNeuronConfig)
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// DataPlastNeuron definition
// for neuron with plastic synapses
// contains a pointer list to the presynapses, and a method for the plasticity update
///////////////////////////////////////////////////////////////////////////////////////////////////

class DataPlastNeuron
        : public DataRecordNeuron
{
    friend class PlasticityManager;
    friend class PlastThread;
    friend class IFMech; //
    friend class InstantIFMech; //
    template <class PoissonParameter, class ActivationFunction> friend class PoissonMech; //
    template <class CorrInputMechConfigType> friend class CorrInputMech;

// construction
protected:
    explicit DataPlastNeuron(ConfigBase * const configurator);
    void add_presynapse_impl(SynapseInterface * const syn);

// update methods
protected:
    void notify_firing_impl(const Time & time_to_spike);
    void plasticity_update();

// members and accessors
protected:
    typedef std::list<SynapseInterface *> ListPreSynType;
    const ListPreSynType & list_presynapses_impl() const {
        return list_presynapses_;
    }
private:
    ListPreSynType list_presynapses_;
    bool has_fired_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// PlasticityManager class definition
// to centralise all plastic neurons and launch the general plasticity update
///////////////////////////////////////////////////////////////////////////////////////////////////

class PlasticityManager
{
    friend class DataPlastNeuron;
    friend class PlastThread;
public:
    static void append(DataPlastNeuron * const pt_nrn);
    static void plast_update_general();
private:
    typedef std::list<DataPlastNeuron *> ListPlastNrnType;
    static ListPlastNrnType list_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// call the plasticity update for all the neurons with the DataPlastNeuron class
//   (supposedly all the pneurons with plastic synapses)
inline void PlasticityManager::plast_update_general()
{
    for (ListPlastNrnType::const_iterator i = list_.begin();
            i != list_.end();
            ++ i)
        (*i)->plasticity_update();
}

// #ifdef PARALLELSIM
// /////////////////////////////////////////////////
// // call the plasticity update for all the neurons with the DataPlastNeuron class
// //   (supposedly all the pneurons with plastic synapses)
// inline void PlasticityManager::par_plast_update_general(ParallelNetManager *const  pnm)
// {
//      for (ListPlastNrnType::const_iterator i = list_.begin();
//       i != list_.end();
//       ++ i)
//      if ( pnm.gid_exists( (*i)->gid ) ) (*i)->plasticity_update();
// }
// #endif

/////////////////////////////////////////////////
// config constructor
inline DataPlastNeuron::DataPlastNeuron(ConfigBase * const configurator)
        : DataRecordNeuron(configurator), has_fired_(false)
{
    if (! dynamic_cast<DataPlastNeuronConfig *>(configurator))
        throw ConfigError("DataPlastNeuron: void configurator");
    PlasticityManager::append(this);
}

/////////////////////////////////////////////////
// called when adding a presynapse to update the information
inline void DataPlastNeuron::add_presynapse_impl(SynapseInterface * const syn)
{
    // add the synapse to the list of presynapses
    list_presynapses_.push_back(syn);
    // add the synaptic mechanism
    DataRecordNeuron::add_presynapse_impl(syn);
}

/////////////////////////////////////////////////
// called when the neuron fires a spike (activation mechanism)
inline void DataPlastNeuron::notify_firing_impl(const Time & time_to_spike)
{
    has_fired_ = true;
    // add the spike time to the list of spike times
    DataRecordNeuron::notify_firing_impl(time_to_spike);
}

/////////////////////////////////////////////////
// called by the neuron after a spike to do the plasticity update on pre- and post-synapses (plasticity mechanism)
inline void DataPlastNeuron::plasticity_update()
{
    // called only when this neuron fired a spike during the timestep
    if (has_fired_) {
        // update all synapses
        for (ListPreSynType::const_iterator i = list_presynapses_impl().begin();
                i != list_presynapses_impl().end();
                ++i)
            (*i)->on_postneuron_fire_plast_update(spike_time_list_impl().back());
        for (ListPostSynType::const_iterator i = list_postsynapses_impl().begin();
                i != list_postsynapses_impl().end();
                ++i)
            (*i)->on_preneuron_fire_plast_update(spike_time_list_impl().back());
        has_fired_ = false;
    }
}

// /////////////////////////////////////////////////
// // called by the neuron after a spike to do the plasticity update on pre- and post-synapses (plasticity mechanism)
// inline void DataPlastNeuron::par_plasticity_update(ParallelNetManager* pnm_)
// {
//  // called only when this neuron fired a spike during the timestep
//  if (has_fired_)
//  {
//      // update all synapses
//      for (ListPreSynType::const_iterator i = list_presynapses_impl().begin();
//              i != list_presynapses_impl().end();
//              ++i)
//          if(pnm_.gid_exists((*i)->gid)) (*i)->on_postneuron_fire_plast_update(spike_time_list_impl().back());
//      for (ListPostSynType::const_iterator i = list_postsynapses_impl().begin();
//              i != list_postsynapses_impl().end();
//              ++i)
//          if(pnm_.gid_exists((*i)->gid)) (*i)->on_preneuron_fire_plast_update(spike_time_list_impl().back());
//      has_fired_ = false;
//  }
// }



#endif // !defined(DATAPLASTNEURON_H)
