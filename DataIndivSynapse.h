// DataIndivSynapse.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DATAINDIVSYNAPSE_H
#define DATAINDIVSYNAPSE_H

#include <deque>

#include "ConfigBase.h"
#include "SimulationEnvironment.h"
#include "DistributionManager.h"
#include "Macros.h"


class DataIndivSynapse;

	
///////////////////////////////////////////////////////////////////////////////////////////////////
// DataIndivSynapseConfig class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

class DataIndivSynapseConfig
	: public ConfigBase
{
public:
	//! Type related to this configurator type.
	/*!	Used for automated construction of neurons from configurators.
		See NeuronFactory and SynapseFactory.
	 */
	typedef DataIndivSynapse related_component;

	//! Accept method for visitor (see class template Visitor).
	MAKE_VISITABLE(DataIndivSynapseConfig)

public:
	DataIndivSynapseConfig(DistributionManager * weightdistrib, DistributionManager * delaydistrib);
	double weight() {return weightdistrib_->generate_value();}
	double delay() {return delaydistrib_->generate_value();}
private:
	DistributionManager * const weightdistrib_ // this class does not own these pointers
					  , * const delaydistrib_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// DataIndivSynapse definition
// for synapses with individual weights and delays, for example:
//  - with a certain distribution on the weights or delays
//  - plastic synapses
///////////////////////////////////////////////////////////////////////////////////////////////////

class DataIndivSynapse
{
	friend class DataLightSynapse;
	friend class DataLightSynapseConfig;
	template <class STDPFunction, class STDPBounds> friend class STDPMech;

// construction
protected:
	explicit DataIndivSynapse(ConfigBase * const configurator);

// members and accessors
	inline double & weight_impl() {return weight_;}
	inline const double & delay_impl() const {return delay_;}
private:
	double weight_
		 , delay_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// config constructor
inline DataIndivSynapse::DataIndivSynapse(ConfigBase * const configurator)
	: weight_(dynamic_cast<DataIndivSynapseConfig *>(configurator)->weight())
	, delay_(dynamic_cast<DataIndivSynapseConfig *>(configurator)->delay())
{
	if ((delay_ < SimEnv::timestep()) || (delay_ > ( SimEnv::i_max_delay() - 1 ) * SimEnv::timestep()))
		throw ConfigError("DataIndivSynapse: delay smaller than the time step or greater than the maximal value (cf. simulation environment SimEnv)");
}






#endif // !defined(DATAINDIVSYNAPSE_H)
