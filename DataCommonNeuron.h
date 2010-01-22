// DataCommonNeuron.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DATACOMMONNEURON_H
#define DATACOMMONNEURON_H


#include <list>
#include <algorithm>
#include <fstream>
#include <boost/shared_ptr.hpp>

#include "Visitor.h"
#include "Error.h"
#include "GlobalDefs.h"
//#include "SimulationEnvironment.h"
#include "ConfigBase.h"
#include "InterfaceBase.h"
#include "Macros.h"


class DataCommonNeuron;
class DataCommonNeuronVisitor;

///////////////////////////////////////////////////////////////////////////////////////////////////
// DataCommonNeuronConfig class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

class DataCommonNeuronConfig
        : public ConfigBase
{
public:
    //! Type related to this configurator type.
    /*! Used for automated construction of neurons from configurators.
        See NeuronFactory and SynapseFactory.
     */
    typedef DataCommonNeuron related_component;

    //! Accept method for visitor (see class template Visitor).
    MAKE_VISITABLE(DataCommonNeuronConfig)
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// DataCommonNeuron definition
// common for all neurons
// contains a pointer list to the link to the presynapses, and a pointer list to the postsynapses
///////////////////////////////////////////////////////////////////////////////////////////////////

class DataCommonNeuron
{
    friend class DataCommonNeuronVisitor;

// construction
protected:
    explicit DataCommonNeuron(ConfigBase * const configurator);
    void add_presynapse_impl(SynapseInterface * const syn);
    void add_postsynapse_impl(SynapseInterface * const syn);
    void apply_vis_impl(AbstractVisitor & vis);

// update methods
protected:
    void notify_firing_impl(const Time & time_to_spike);
    inline void plast_update_impl() {}

// members and accessors
protected:
    typedef std::list<boost::shared_ptr<SynMechInterface> > ListSynMechType;
    typedef std::list<boost::shared_ptr<SynapseInterface> > ListPostSynType;
    const ListSynMechType & presynmechlist_impl() const {
        return presyn_mech_list_;
    };
    const ListPostSynType & list_postsynapses_impl() const {
        return list_postsynapses_;
    } // list of postsynapses
private:
    ListSynMechType presyn_mech_list_; // list of presynaptic mechanisms
    ListPostSynType list_postsynapses_; // list of postsynapses
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// DataCommonNeuronVisitor class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

class DataCommonNeuronVisitor
        : public AbstractVisitor
        , public Visitor<DataCommonNeuron>
{
public:
    typedef DataCommonNeuron::ListPostSynType ListPostSynType;
    DataCommonNeuronVisitor() : postsyn_list_pointer_(0) {}
    void reset() {
        postsyn_list_pointer_ = 0;
    }
    void visit(DataCommonNeuron & dcn) {
        postsyn_list_pointer_ = & dcn.list_postsynapses_;
    }
    ListPostSynType * const postsyn_list_pointer() const {
        return postsyn_list_pointer_;
    }
protected:
    ListPostSynType * postsyn_list_pointer_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////


//! constructor
inline DataCommonNeuron::DataCommonNeuron(ConfigBase * const configurator)
{
    if (! dynamic_cast<DataCommonNeuronConfig *>(configurator))
        throw ConfigError("DataCommonNeuron: void configurator");
};


//! add a preneuron to the list
inline void DataCommonNeuron::add_presynapse_impl(SynapseInterface * const syn)
{
    // add the synaptic mechanism if it is not already in the list (no duplicate)
    bool found = false;
    for (ListSynMechType::const_iterator i = presyn_mech_list_.begin(); i != presyn_mech_list_.end(); ++i)
        if (syn->syn_mech() == (*i).operator->()) {
            found = true;
            break;
        }
    if (! found)
        presyn_mech_list_.push_back(boost::shared_ptr<SynMechInterface>(syn->syn_mech()));
}


//! add a postneuron to the list
inline void DataCommonNeuron::add_postsynapse_impl(SynapseInterface * const syn)
{
    list_postsynapses_.push_back(boost::shared_ptr<SynapseInterface>(syn));
}


//! called when the neuron fires a spike (activation mechanism)
inline void DataCommonNeuron::notify_firing_impl(const Time & time_to_spike)
{
//TODO add parallel send or
    // notify the post synapses
    for (ListPostSynType::const_iterator i = list_postsynapses_.begin();
            i != list_postsynapses_.end();
            ++i)
        (*i)->on_preneuron_fire_update(time_to_spike);
}


//! to access the list of postsynapses using Vistor
inline void DataCommonNeuron::apply_vis_impl(AbstractVisitor & vis)
{
    // check the compatibilty of the visitor and call the method visit if suitable
    DataCommonNeuronVisitor * ptvis = dynamic_cast<DataCommonNeuronVisitor *>(& vis);
    if (ptvis)
        ptvis->visit(*this);
}



#endif // !defined(DATACOMMONNEURON_H)
