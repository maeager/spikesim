// ConcreteSynapseFactory.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CONCRETESYNAPSEFACTORY_H
#define CONCRETESYNAPSEFACTORY_H


#include "SynapseFactory.h"



//! AbstractSynapseFactory: Abstract class for all synapses factories.
/*! Generic interface for all derived class templates ConcreteSynapseFactory
	(which are built with the real classes of mechanisms)
 */
struct AbstractSynapseFactory
{
	virtual void create(const std::list<NeuronInterface *> & preneuron_list, NeuronInterface * const postneuron) = 0;
};



//! ConcreteSynapseFactory: 
/*! Class template with real configurators (DataConfig, SynMechConfig, PlastMechConfig).
	These classes are called by SynapseFactory to effectively build the actual synapses
	according to the mechanisms.
 */
template <class DataConfig, class SynMechConfig, class PlastMechConfig> 
class ConcreteSynapseFactory
	: public AbstractSynapseFactory
{
public:
	typedef typename SynapseTypeMap<typename DataConfig::related_component
								  , typename SynMechConfig::related_component
								  , typename PlastMechConfig::related_component
								   >::created_type
			SynapseTypeToCreate;
	typedef typename SynMechConfig::related_component SynMechTypeToCreate;

public:
	ConcreteSynapseFactory(DataConfig * const dpcfg, SynMechConfig * const smpcfg, PlastMechConfig * const pmpcfg)
		: data_cfg_(dpcfg)
		, syn_mech_cfg_(smpcfg)
		, plast_mech_cfg_(pmpcfg)
	{}

	void create(const std::list<NeuronInterface *> & preneuron_list, NeuronInterface * const postneuron)
	{
		// create a new activation synaptic mechanism that will be shared by all the created synapses
		SynMechTypeToCreate * syn_mech = new SynMechTypeToCreate(syn_mech_cfg_); 
		// creation of the synapses, one per preneuron from the list
		for (std::list<NeuronInterface *>::const_iterator i = preneuron_list.begin();
			 i != preneuron_list.end();
			 ++i)
			SynapseTypeToCreate * temp = new SynapseTypeToCreate(data_cfg_, syn_mech, plast_mech_cfg_, *i, postneuron);
	}

protected:
	DataConfig * const data_cfg_;
	SynMechConfig * const syn_mech_cfg_;
	PlastMechConfig * const plast_mech_cfg_;
};



// ConcreteSynapseFactory class template specialisation for DataPlastSynapseConfig
template <class SynMechConfig, class PlastMechConfig> 
class ConcreteSynapseFactory<DataPlastSynapseConfig, SynMechConfig, PlastMechConfig> 
	: public AbstractSynapseFactory
{
public:
	typedef typename SynapseTypeMap<typename DataPlastSynapseConfig::related_component
								  , typename SynMechConfig::related_component
								  , typename PlastMechConfig::related_component
								   >::created_type
			SynapseTypeToCreate;
	typedef typename SynMechConfig::related_component SynMechTypeToCreate;

public:
	ConcreteSynapseFactory(DataPlastSynapseConfig * const dpcfg, SynMechConfig * const smpcfg, PlastMechConfig * const pmpcfg)
		: data_cfg_(dpcfg)
		, syn_mech_cfg_(smpcfg)
		, plast_mech_cfg_(pmpcfg)
	{}

	void create(const std::list<NeuronInterface *> & preneuron_list, NeuronInterface * const postneuron)
	{
		// create a new activation synaptic mechanism that will be shared by all the created synapses
		SynMechTypeToCreate * syn_mech = new SynMechTypeToCreate(syn_mech_cfg_); 
		// creation of the synapses, one per preneuron from the list
		for (std::list<NeuronInterface *>::const_iterator i = preneuron_list.begin();
			 i != preneuron_list.end();
			 ++i)
		{
			data_cfg_->get_spike_list_from_pre_and_postneurons(*i, postneuron);
			SynapseTypeToCreate * temp = new SynapseTypeToCreate(data_cfg_, syn_mech, plast_mech_cfg_, *i, postneuron);
		}
	}

protected:
	DataPlastSynapseConfig * const data_cfg_;
	SynMechConfig * const syn_mech_cfg_;
	PlastMechConfig * const plast_mech_cfg_;
};



#endif // !defined(CONCRETESYNAPSEFACTORY_H)
