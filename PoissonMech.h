// PoissonMech.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef POISSONMECH_H
#define POISSONMECH_H

#include "SimulationEnvironment.h"
#include "ConfigBase.h"
#include "Error.h"
#include "Macros.h"
#include "RandomGenerator.h"
#include "NeuronActivationFunction.h"
#include "SpikeTimeGenerator.h"


template <class PoissonParameter, class ActivationFunction> class PoissonMech;


///////////////////////////////////////////////////////////////////////////////////////////////////
// PoissonMechConfig class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

template < class PoissonParameter, class ActivationFunction = FuncIdentity >
class PoissonMechConfig
        : public ConfigBase
        , public PoissonParameter
{
public:
    //! Type related to this configurator type.
    /*! Used for automated construction of neurons from configurators.
        See NeuronFactory and SynapseFactory.
     */
    typedef PoissonMech<PoissonParameter, ActivationFunction> related_component;

    //! Accept method for visitor (see class template Visitor).
    MAKE_VISITABLE(PoissonMechConfig)

public:
    PoissonMechConfig(std::ifstream & is);
    inline const double & spontaneous_rate() const {
        return PoissonParameter::spontaneous_rate_;
    }
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// PoissonMech definition
// models a Poisson neuron
///////////////////////////////////////////////////////////////////////////////////////////////////

template <class PoissonParameter, class ActivationFunction>
class PoissonMech
{
protected:
    explicit PoissonMech(ConfigBase * const configurator);
    template <class TypeImpl> void perform_calculations(TypeImpl & neuron);
    const Volt & potential_impl() const {
        return rate_;
    }
private:
    PoissonMechConfig<PoissonParameter, ActivationFunction> * const pmcfg_;
    double rate_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// PoissonMech inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// constructor
template <class PoissonParameter, class ActivationFunction>
PoissonMech<PoissonParameter, ActivationFunction>::PoissonMech(ConfigBase * const configurator)
        : pmcfg_(dynamic_cast<PoissonMechConfig<PoissonParameter, ActivationFunction> *>(configurator))
        , rate_(0.)
{
    if (! pmcfg_) throw ConfigError("PoissonMech: void configurator");
}

/////////////////////////////////////////////////
// mechanism called to update the state of the neuron
template <class PoissonParameter, class ActivationFunction>
template <class TypeImpl>
inline void PoissonMech<PoissonParameter, ActivationFunction>::perform_calculations(TypeImpl & neuron)
{
    // reset the rate
    rate_ = pmcfg_->spontaneous_rate();

    // sum all synaptic contributions
    for (typename TypeImpl::ListSynMechType::const_iterator i = neuron.presynmechlist_impl().begin();
            i != static_cast<TypeImpl &>(*this).presynmechlist_impl().end();
            ++i)
        (*i)->send_updated_states(rate_);

    // apply the activation function
    rate_ = ActivationFunction::func(rate_);

    if (RandomGenerator::probability_trial_for_time_step(rate_))  // presentrate in  Hz.
        static_cast<TypeImpl &>(*this).notify_firing_impl(0);
//      static_cast<TypeImpl &>(*this).notify_firing_impl( SpikeTimeGenerator::gen_.generate_value() );
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// PoissonMechConfig function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// PoissonMechConfig constructor with script file
template <class PoissonParameter, class ActivationFunction>
PoissonMechConfig<PoissonParameter, ActivationFunction>::PoissonMechConfig(std::ifstream & is)
        : PoissonParameter(is)
{
}

/////////////////////////////////////////////////
// stream operator overloading
template <class PoissonParameter, class ActivationFunction>
std::ofstream & operator<<(std::ofstream & os, const PoissonMechConfig<PoissonParameter, ActivationFunction> & pmcfg)
{
    os << "spontaneous_rate " << pmcfg.spontaneous_rate()
    << ";";
    return os;
}

#endif // !defined(POISSONMECH_H)
