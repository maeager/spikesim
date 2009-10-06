// NeuronFactory.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NEURONFACTORY_H
#define NEURONFACTORY_H


#include "TypeDefs.h"
#include "ConfigBase.h"
#include "InterfaceBase.h"
#include "Dispatcher.h"
#include "CreatedTypeMaps.h"



//! Factory used to create a neuron given configurators (data, activation mechanism, etc.).
/*! This class derives from a visitor that can be applied on the configurators to make a double dispatch and
    retrieve the right neuron type to create.
    The mechanism configurators are passed to the constructor of the factory and the method 'create'
    returns a pointer to a newly created neuron that is made with the given configurators.
 */
class NeuronFactory
{
    template <class Configurator> friend struct DispatcherNrn2;

public:
    //! Constructor.
    /*! \param act_mech_cfg configurator for the activation mechanism (Poisson, I-&-F, etc.)
        \param data_cfg configurator for the neural data (pre- and post- synapses, history spike list, etc.)
     */
    NeuronFactory(ConfigBase * data_cfg, ConfigBase * act_mech_cfg);

    //! Destructor.
    ~NeuronFactory() {
        delete concrete_factory_;
    }

    //! Returns a const pointer to a newly created neuron.
    /*! The creation is done according to the given configurators.
     */
    NeuronInterface * const create();

protected:
    AbstractFactory<NeuronInterface> * concrete_factory_; /*!< Base pointer to the concrete factory (with the real types) to create the neurons. */
};



//! Wrapper of real types of neuron configurators, used by NeuronFactory to create new neurons.
/*! This class derives from a visitor that can be applied on the configurators to make a double dispatch and
    retrieve the right neuron type to create.
    The mechanism configurators are passed to the constructor of the factory and the method 'create'
    returns a pointer to a newly created neuron that is made with the given configurators.
 */
template <class DataConfig, class ActMechConfig>
class NeuronFactoryHelper
        : public AbstractFactory<NeuronInterface>
{
    //! Real type of neuron to create.
    typedef typename NeuronTypeMap < typename DataConfig::related_component
    , typename ActMechConfig::related_component
    >::created_type
    type_to_create;

    //! Type constructor to create pointer to a new neuron.
    /*! To eliminate the constructor of NullType and be able to provide specialisations certain type.
     */
    template <class TypeToCreate>
    struct TypeConstructor {
        static NeuronInterface * const create_impl(DataConfig * const data_cfg, ActMechConfig * const act_mech_cfg) {
            return new TypeToCreate(data_cfg, act_mech_cfg);
        }
    };

public:
    //! Constructor
    /*! \param data_cfg real type of neural data configurator.
        \param act_mech_cfg real type of neural activation mechanism configurator.
     */
    NeuronFactoryHelper(DataConfig * const data_cfg, ActMechConfig * const act_mech_cfg) : data_cfg_(data_cfg), act_mech_cfg_(act_mech_cfg) {}

    //! Returns a pointer to a newly created neuron.
    NeuronInterface * const create() {
        return TypeConstructor<type_to_create>::create_impl(data_cfg_, act_mech_cfg_);
    }

private:
    DataConfig * const data_cfg_; /*!< Configurator for the type of data for the neuron to create.*/
    ActMechConfig * const act_mech_cfg_; /*!< Configurator for the activation mechanism for the neuron to create.*/
};




template <class Configurator>
struct DispatcherNrn2
            : public AbstractVisitor
            , public Visitor<IFMechConfig>
            , public Visitor<InstantIFMechConfig>
            , public Visitor<PoissonMechConfig<ConstantPoissonParameter, FuncIdentity> >
            , public Visitor<PoissonMechConfig<ConstantPoissonParameter, FuncSigmoid> >
            , public Visitor<PoissonMechConfig<OscillatoryPoissonParameter, FuncIdentity> >
            , public Visitor<PoissonMechConfig<OscillatoryPoissonParameter, FuncSigmoid> >
            , public Visitor<DeltaCorrInputRef>
            , public Visitor<CorrInputRefShiftedCopy>
            , public Visitor<ShortTimeCorrInputRef> {
    DispatcherNrn2(Configurator * cfg1, NeuronFactory * nrn_fact) : cfg1_(cfg1), nrn_fact_(nrn_fact) {}
    void visit(IFMechConfig & cfg2) {
        nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator, IFMechConfig>(cfg1_, & cfg2);
    }
    void visit(InstantIFMechConfig & cfg2) {
        nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator, InstantIFMechConfig>(cfg1_, & cfg2);
    }
    void visit(PoissonMechConfig<ConstantPoissonParameter, FuncIdentity> & cfg2)  {
        nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator, PoissonMechConfig<ConstantPoissonParameter, FuncIdentity> >(cfg1_, & cfg2);
    }
    void visit(PoissonMechConfig<ConstantPoissonParameter, FuncSigmoid> & cfg2) {
        nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator, PoissonMechConfig<ConstantPoissonParameter, FuncSigmoid> >(cfg1_, & cfg2);
    }
    void visit(PoissonMechConfig<OscillatoryPoissonParameter, FuncIdentity> & cfg2)  {
        nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator, PoissonMechConfig<OscillatoryPoissonParameter, FuncIdentity> >(cfg1_, & cfg2);
    }
    void visit(PoissonMechConfig<OscillatoryPoissonParameter, FuncSigmoid> & cfg2) {
        nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator, PoissonMechConfig<OscillatoryPoissonParameter, FuncSigmoid> >(cfg1_, & cfg2);
    }
    void visit(DeltaCorrInputRef & cfg2) {
        nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator, DeltaCorrInputRef>(cfg1_, & cfg2);
    }
    void visit(CorrInputRefShiftedCopy & cfg2) {
        nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator, CorrInputRefShiftedCopy>(cfg1_, & cfg2);
    }
    void visit(ShortTimeCorrInputRef & cfg2) {
        nrn_fact_->concrete_factory_ = new NeuronFactoryHelper<Configurator, ShortTimeCorrInputRef>(cfg1_, & cfg2);
    }
    Configurator * cfg1_; // holds memory of real-type (already dispatched) configurator object
    NeuronFactory * nrn_fact_;
};



struct DispatcherNrn1
            : public AbstractVisitor
            , public Visitor<DataCommonNeuronConfig>
            , public Visitor<DataRecordNeuronConfig>
            , public Visitor<DataPlastNeuronConfig> {
    DispatcherNrn1(ConfigBase * data_cfg, ConfigBase * act_mech_cfg, NeuronFactory * nrn_fact)
            : data_cfg_(data_cfg), act_mech_cfg_(act_mech_cfg), nrn_fact_(nrn_fact)  {}
    void op() {
        data_cfg_->apply_vis(*this);
    }
    void visit(DataCommonNeuronConfig & cfg) {
        DispatcherNrn2<DataCommonNeuronConfig> dips2(&cfg, nrn_fact_);
        act_mech_cfg_->apply_vis(dips2);
    }
    void visit(DataRecordNeuronConfig & cfg) {
        DispatcherNrn2<DataRecordNeuronConfig> dips2(&cfg, nrn_fact_);
        act_mech_cfg_->apply_vis(dips2);
    }
    void visit(DataPlastNeuronConfig & cfg) {
        DispatcherNrn2<DataPlastNeuronConfig> dips2(&cfg, nrn_fact_);
        act_mech_cfg_->apply_vis(dips2);
    }
    ConfigBase * data_cfg_, * act_mech_cfg_;
    NeuronFactory * nrn_fact_;
};





///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// NeuronFactory constructor
inline NeuronFactory::NeuronFactory(ConfigBase * const data_cfg, ConfigBase * const act_mech_cfg)
{
    DispatcherNrn1 disp(data_cfg, act_mech_cfg, this);
    disp.op();
}

/////////////////////////////////////////////////
// create method
// calls the method create_impl with the real types of the configurators
// and returns a pointer to the newly created object
inline NeuronInterface * const NeuronFactory::create()
{
    // returns a pointer to the newly created neuron
    return concrete_factory_->create();
}



#endif // !defined(NEURONFACTORY_H)
