// SynapseFactory.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SYNAPSEFACTORY_H
#define SYNAPSEFACTORY_H


#include "TypeDefs.h"
#include "ConcreteSynapseFactory.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// SynapseFactory class definition
// creates a synapse according to the distribution of weights and delays, synaptic mechanism, plasticity mechanism
///////////////////////////////////////////////////////////////////////////////////////////////////

class SynapseFactory
{
public:
	SynapseFactory(DistributionManager * const weight_distrib_cfg, DistributionManager * const delay_distrib_cfg, ConfigBase * const syn_mech_cfg, ConfigBase * const plast_mech_cfg, std::list<boost::shared_ptr<ConfigBase> > & cfglist);
	void create(const std::list<NeuronInterface *> & preneuron_list, NeuronInterface * const postneuron)
	{
		if (concrete_factory_)
			concrete_factory_->create(preneuron_list, postneuron);
		else
			throw ConfigError("SynapseFactory: error creating synapses");
	}
protected:
	AbstractSynapseFactory * concrete_factory_;
	ConfigBase * data_cfg_; // data configurator
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// SynapseFactory constructor
// determines the correct type of synapse to create according to the given configurators in argument
inline SynapseFactory::SynapseFactory(DistributionManager * const weight_distrib_cfg
									, DistributionManager * const delay_distrib_cfg
									, ConfigBase * const syn_mech_cfg
									, ConfigBase * const plast_mech_cfg
									, std::list<boost::shared_ptr<ConfigBase> > & cfglist)
	: concrete_factory_(0)
{
	// possible distribution configurators for the synaptic weight
	DeltaDistribution * ddswc = dynamic_cast<DeltaDistribution *>(weight_distrib_cfg);
	UniformDistribution * udswc = dynamic_cast<UniformDistribution *>(weight_distrib_cfg);
	GaussianDistribution * gdswc = dynamic_cast<GaussianDistribution *>(weight_distrib_cfg);

	// possible distribution configurators for the synaptic delay
	DeltaDistribution * ddsdc = dynamic_cast<DeltaDistribution *>(delay_distrib_cfg);
	UniformDistribution * udsdc = dynamic_cast<UniformDistribution *>(delay_distrib_cfg);
	GaussianDistribution * gdsdc = dynamic_cast<GaussianDistribution *>(delay_distrib_cfg);

	// possible synaptic activation mechanism configurators
	DoubleExpSynMechConfig * asmsc = dynamic_cast<DoubleExpSynMechConfig *>(syn_mech_cfg);
	CondSynMechConfig * csmsc = dynamic_cast<CondSynMechConfig *>(syn_mech_cfg);
	InstantSynMechConfig * ismsc = dynamic_cast<InstantSynMechConfig *>(syn_mech_cfg);

	// possible plasticity mechanism configurators
	NoPlastMechConfig * npmsc = dynamic_cast<NoPlastMechConfig *>(plast_mech_cfg);
	STDPMechConfig<AdditiveSTDPNegExpFunction, ClipToBounds> * stdpmsc = dynamic_cast<STDPMechConfig<AdditiveSTDPNegExpFunction, ClipToBounds> *>(plast_mech_cfg);

	// creation of the corresponding concrete factory
	if (csmsc) {
		if (npmsc) {
			if (ddswc && ddsdc) {
				data_cfg_ = new DataLightSynapseConfig(weight_distrib_cfg, delay_distrib_cfg);
				cfglist.push_back(boost::shared_ptr<ConfigBase>(data_cfg_));
				concrete_factory_ = new ConcreteSynapseFactory<DataLightSynapseConfig, CondSynMechConfig, NoPlastMechConfig>(dynamic_cast<DataLightSynapseConfig *>(data_cfg_), csmsc, npmsc);
			} else {
				data_cfg_ = new DataIndivSynapseConfig(weight_distrib_cfg, delay_distrib_cfg);
				concrete_factory_ = new ConcreteSynapseFactory<DataIndivSynapseConfig, CondSynMechConfig, NoPlastMechConfig>(dynamic_cast<DataIndivSynapseConfig *>(data_cfg_), csmsc, npmsc);
			}
		} else if (stdpmsc) {
			data_cfg_ = new DataPlastSynapseConfig(weight_distrib_cfg, delay_distrib_cfg);
			concrete_factory_ = new ConcreteSynapseFactory<DataPlastSynapseConfig, CondSynMechConfig, STDPMechConfig<AdditiveSTDPNegExpFunction, ClipToBounds> >(dynamic_cast<DataPlastSynapseConfig *>(data_cfg_), csmsc, stdpmsc);
		} else {
			throw ConfigError("SynapseFactory: unknown plasticity mechanism configurator");
		}
	} else if (ismsc) {
		if (npmsc) {
			if (ddswc && ddsdc) {
				data_cfg_ = new DataLightSynapseConfig(weight_distrib_cfg, delay_distrib_cfg);
				cfglist.push_back(boost::shared_ptr<ConfigBase>(data_cfg_));
				concrete_factory_ = new ConcreteSynapseFactory<DataLightSynapseConfig, InstantSynMechConfig, NoPlastMechConfig>(dynamic_cast<DataLightSynapseConfig *>(data_cfg_), ismsc, npmsc);
			} else {
				data_cfg_ = new DataIndivSynapseConfig(weight_distrib_cfg, delay_distrib_cfg);
				concrete_factory_ = new ConcreteSynapseFactory<DataIndivSynapseConfig, InstantSynMechConfig, NoPlastMechConfig>(dynamic_cast<DataIndivSynapseConfig *>(data_cfg_), ismsc, npmsc);
			}
		} else if (stdpmsc) {
			data_cfg_ = new DataPlastSynapseConfig(weight_distrib_cfg, delay_distrib_cfg);
			concrete_factory_ = new ConcreteSynapseFactory<DataPlastSynapseConfig, InstantSynMechConfig, STDPMechConfig<AdditiveSTDPNegExpFunction, ClipToBounds> >(dynamic_cast<DataPlastSynapseConfig *>(data_cfg_), ismsc, stdpmsc);
		} else {
			throw ConfigError("SynapseFactory: unknown plasticity mechanism configurator");
		}
	} else if (asmsc) {
		if (npmsc) {
			if (ddswc && ddsdc) {
				data_cfg_ = new DataLightSynapseConfig(weight_distrib_cfg, delay_distrib_cfg);
				cfglist.push_back(boost::shared_ptr<ConfigBase>(data_cfg_));
				concrete_factory_ = new ConcreteSynapseFactory<DataLightSynapseConfig, DoubleExpSynMechConfig, NoPlastMechConfig>(dynamic_cast<DataLightSynapseConfig *>(data_cfg_), asmsc, npmsc);
			} else {
				data_cfg_ = new DataIndivSynapseConfig(weight_distrib_cfg, delay_distrib_cfg);
				concrete_factory_ = new ConcreteSynapseFactory<DataIndivSynapseConfig, DoubleExpSynMechConfig, NoPlastMechConfig>(dynamic_cast<DataIndivSynapseConfig *>(data_cfg_), asmsc, npmsc);
			}
		} else if (stdpmsc) {
			data_cfg_ = new DataPlastSynapseConfig(weight_distrib_cfg, delay_distrib_cfg);
			concrete_factory_ = new ConcreteSynapseFactory<DataPlastSynapseConfig, DoubleExpSynMechConfig, STDPMechConfig<AdditiveSTDPNegExpFunction, ClipToBounds> >(dynamic_cast<DataPlastSynapseConfig *>(data_cfg_), asmsc, stdpmsc);
		} else {
			throw ConfigError("SynapseFactory: unknown plasticity mechanism configurator");
		}
	} else {
		throw ConfigError("SynapseFactory: unknown synaptic activation mechanism configurator");
	}

//	Dispatcher<>

}


/*
xxx
template <class Configurator> 
struct DispatcherSyn2
	: public AbstractVisitor
	, public Visitor<IFMechConfig>
	, public Visitor<InstantIFMechConfig>
	, public Visitor<PoissonMechConfig<ConstantPoissonParameter, FuncIdentity> >
	, public Visitor<PoissonMechConfig<ConstantPoissonParameter, FuncSigmoid> >
	, public Visitor<PoissonMechConfig<OscillatoryPoissonParameter, FuncIdentity> >
	, public Visitor<PoissonMechConfig<OscillatoryPoissonParameter, FuncSigmoid> >
	, public Visitor<DeltaCorrInputRef>
	, public Visitor<CorrInputRefShiftedCopy>
	, public Visitor<ShortTimeCorrInputRef>
{
	DispatcherNrn2(Configurator * cfg1, NeuronFactory * nrn_fact) : cfg1_(cfg1), nrn_fact_(nrn_fact) {}
	void visit(IFMechConfig & cfg2) {nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator,IFMechConfig>(cfg1_, & cfg2);}
	void visit(InstantIFMechConfig & cfg2) {nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator,InstantIFMechConfig>(cfg1_, & cfg2);}
	void visit(PoissonMechConfig<ConstantPoissonParameter, FuncIdentity> & cfg2)  {nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator,PoissonMechConfig<ConstantPoissonParameter, FuncIdentity> >(cfg1_, & cfg2);}
	void visit(PoissonMechConfig<ConstantPoissonParameter, FuncSigmoid> & cfg2) {nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator,PoissonMechConfig<ConstantPoissonParameter, FuncSigmoid> >(cfg1_, & cfg2);}
	void visit(PoissonMechConfig<OscillatoryPoissonParameter, FuncIdentity> & cfg2)  {nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator,PoissonMechConfig<OscillatoryPoissonParameter, FuncIdentity> >(cfg1_, & cfg2);}
	void visit(PoissonMechConfig<OscillatoryPoissonParameter, FuncSigmoid> & cfg2) {nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator,PoissonMechConfig<OscillatoryPoissonParameter, FuncSigmoid> >(cfg1_, & cfg2);}
	void visit(DeltaCorrInputRef & cfg2) {nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator,DeltaCorrInputRef>(cfg1_, & cfg2);}
	void visit(CorrInputRefShiftedCopy & cfg2) {nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator,CorrInputRefShiftedCopy>(cfg1_, & cfg2);}
	void visit(ShortTimeCorrInputRef & cfg2) {nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator,ShortTimeCorrInputRef>(cfg1_, & cfg2);}
	Configurator * cfg1_; // holds memory of real-type (already dispatched) configurator object
	NeuronFactory * nrn_fact_;
};



struct DispatcherSyn1
	: public AbstractVisitor
	, public Visitor<DataLightSynapseConfig>
	, public Visitor<DataLightSynapseConfig>
	, public Visitor<DataLightSynapseConfig>
{
	DispatcherNrn1(ConfigBase * data_cfg, ConfigBase * act_mech_cfg, NeuronFactory * nrn_fact) 
		: data_cfg_(data_cfg), act_mech_cfg_(act_mech_cfg), nrn_fact_(nrn_fact)  {}
	void op() {data_cfg_->apply_vis(*this);}
	void visit(DataCommonNeuronConfig & cfg)
	{
		DispatcherNrn2<DataCommonNeuronConfig> dips2(&cfg, nrn_fact_);
		act_mech_cfg_->apply_vis(dips2);
	}
	void visit(DataRecordNeuronConfig & cfg)
	{
		DispatcherNrn2<DataRecordNeuronConfig> dips2(&cfg, nrn_fact_);
		act_mech_cfg_->apply_vis(dips2);
	}
	void visit(DataPlastNeuronConfig & cfg)
	{
		DispatcherNrn2<DataPlastNeuronConfig> dips2(&cfg, nrn_fact_);
		act_mech_cfg_->apply_vis(dips2);
	}
	ConfigBase * data_cfg_, * act_mech_cfg_;
	NeuronFactory * nrn_fact_;
};
*/
#endif // !defined(SYNAPSEFACTORY_H)
