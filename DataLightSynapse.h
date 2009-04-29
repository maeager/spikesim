// DataLightSynapse.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DATALIGHTSYNAPSE_H
#define DATALIGHTSYNAPSE_H


#include "ConfigBase.h"
#include "DataIndivSynapse.h"
#include "Macros.h"


class DataLightSynapse;


///////////////////////////////////////////////////////////////////////////////////////////////////
// DataLightSynapseConfig class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

class DataLightSynapseConfig
	: public ConfigBase
{
public:
	//! Type related to this configurator type.
	/*!	Used for automated construction of neurons from configurators.
		See NeuronFactory and SynapseFactory.
	 */
	typedef DataLightSynapse related_component;

	//! Accept method for visitor (see class template Visitor).
	MAKE_VISITABLE(DataLightSynapseConfig)

public:
	DataLightSynapseConfig(DistributionManager * weightdistrib, DistributionManager * delaydistrib);
	inline const double & weight() const {return weight_;}
	inline const double & delay() const {return delay_;}
private:
	const double weight_
			   , delay_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// DataLightSynapse definition
// for synapses that share same weights and delays
// contains a reference to the common data
///////////////////////////////////////////////////////////////////////////////////////////////////

class DataLightSynapse
{
// construction
protected:
	explicit DataLightSynapse(ConfigBase * const configurator);

// members and accessors
protected:
	inline const double & weight_impl() const {return data_ref_->weight();}
	inline const double & delay_impl() const {return data_ref_->delay();}
private:
	const DataLightSynapseConfig * const data_ref_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// config constructor
inline DataLightSynapse::DataLightSynapse(ConfigBase * const configurator)
	: data_ref_(dynamic_cast<DataLightSynapseConfig *>(configurator))
{
	if (! data_ref_) throw ConfigError("DataLightSynapse: data_ref_ not initialised");
}




#endif // !defined(DATALIGHTSYNAPSE_H)
