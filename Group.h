// Group.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GROUP_H
#define GROUP_H

#include <list>
#include <fstream>
#include <string>
#include<boost/shared_ptr.hpp>

#include "IdCounter.h"
#include "Error.h"
#include "DataCommonNeuron.h"
#include "DataRecordNeuron.h"
#include "TypeDefs.h"
#include "GlobalDefs.h"
#include "ConnectivityManager.h"
#include "DistributionManager.h"


//! Group of neurons sharing similar properties.
/*!	All the neurons of the group share the same configurators for the neural data (\link Group::data_cfg_ \endlink) 
	and activation (\link Group::nrn_act_cfg_ \endlink) mechanisms.
*/
class Group
//	: public IdCounter<Group>
{
	friend class SingleWayThroughGroups;
	friend class AllCrossPairsThroughSameGroups;
	friend class Network;
public:
	Group() : data_cfg_(0), nrn_act_cfg_(0) {}
	void populate(std::ifstream & is);
	~Group();
	void connect_to(Group & targetgroup, DistributionManager * const weightdistribcfg, DistributionManager * const delaydistribcfg, ConfigBase * const synmechcfg, ConfigBase * const plastmechcfg, ConnectivityManager * const connectivitymgr, std::list<boost::shared_ptr<ConfigBase> > & cfglist, Size & nb_con);
	void update();
//	void plasticity_update();

// a virer ??? xxx
	typedef std::list<boost::shared_ptr<NeuronInterface> > ListNrnType; /*!< Type redefinition for the list of pointers to the neurons. */
	ListNrnType list_; /*!< List of pointers to the neurons of the group. */

private:
	ConfigBase * data_cfg_; /*!< Common configurator for the neural data shared by all the neurons of this group. */
	ConfigBase * nrn_act_cfg_; /*!< Common configurator for the activation mechanism shared by all the neurons of this group. */



// accessors to information about the group
public:
	const ConfigBase * const neuronconfigurator() const {return nrn_act_cfg_;}
	Size size() const {return (Size) list_.size();}
	void clear_past_of_spike_list(const Time & time_end_past);
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Group inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Update method
// all the groups are updates from #0 to last one
inline void Group::update()
{
	for (ListNrnType::iterator i = list_.begin(); 
		 i != list_.end(); 
		 ++i)
		(*i)->update();
}

/////////////////////////////////////////////////
// clean the spike list of the neurons up to 'time_end_past' (excluded)
inline void Group::clear_past_of_spike_list(const Time & time_end_past)
{
	DataRecordNeuronVisitor vis;
	for (ListNrnType::const_iterator i = list_.begin(); i != list_.end(); ++i)
	{
		(*i)->apply_visitor(vis);
		//
		if (vis.spike_time_list_pointer())
		{
			std::deque<Time>::iterator j = vis.spike_time_list_pointer()->begin();
			for ( ; j != vis.spike_time_list_pointer()->end();
				 ++j) 
				if ((*j) > time_end_past) break;
			vis.erase_past_spike_list(j);
		}
		//
		vis.reset();
	}
}

#endif // !defined(GROUP_H)
