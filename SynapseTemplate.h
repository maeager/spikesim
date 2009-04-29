// SynapseTemplate.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SYNAPSETEMPLATE_H
#define SYNAPSETEMPLATE_H


#include "ConfigBase.h"
#include "Error.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// SynapseTemplate class template definition
// data (weight, delay, etc) are in SynData
// the activation update mechanism is dealt with by SynMech
// the plasticity update mechanism is dealt with by PlastMech
// the interface is TypeBase
///////////////////////////////////////////////////////////////////////////////////////////////////

template <class SynData
	, class SynMech
	, class PlastMech
	, class TypeBase>
class SynapseTemplate
	: public SynData
	, public PlastMech
	, public TypeBase
{
	friend class Bide<SynData>::X;
	friend class Bide<PlastMech>::X;
	friend class Bide<SynMech>::X;
	
	friend class DataCommonNeuron;
public:
	SynapseTemplate(ConfigBase * datacfg, SynMech * const syn_mech, ConfigBase * plast_cfg, NeuronInterface * const preneuron, NeuronInterface * const postneuron);
	inline const double & weight() {return SynData::weight_impl();}
	inline const double & delay() {return SynData::delay_impl();}
protected:
	inline void on_preneuron_fire_update(const double & time_lag_to_spike)
	{
		syn_mech_->on_preneuron_fire_update(time_lag_to_spike + SynData::delay_impl(), SynData::weight_impl());
	}
	inline void on_preneuron_fire_plast_update(const double & preneuron_spike_time)
	{
		PlastMech::on_preneuron_fire_plast_update(*this, preneuron_spike_time);
	}
	inline void on_postneuron_fire_plast_update(const double & postneuron_spike_time)
	{
		PlastMech::on_postneuron_fire_plast_update(*this, postneuron_spike_time);
	}
	inline SynMechInterface * const syn_mech() const {return syn_mech_;}
	SynMech * const syn_mech_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definition
///////////////////////////////////////////////////////////////////////////////////////////////////

template <class SynData
	, class SynMech
	, class PlastMech
	, class TypeBase>
SynapseTemplate<SynData, SynMech, PlastMech, TypeBase>::SynapseTemplate(
			ConfigBase * datacfg,
			SynMech * const synmech,
			ConfigBase * plastcfg,
			NeuronInterface * const preneuron,
			NeuronInterface * const postneuron)
	: TypeBase(postneuron)
	, SynData(datacfg)
	, PlastMech(plastcfg)
	, syn_mech_(synmech)
{
	if (preneuron) preneuron->add_postsynapse(this);
	if (postneuron)
		postneuron->add_presynapse(this);
	else
		throw ConfigError("SynapseTemplate: null pointer to postneuron");
// bide
//	std::cout << preneuron->id() << "  " << postneuron->id() << std::endl;
}




#endif // !defined(SYNAPSETEMPLATE_H)
