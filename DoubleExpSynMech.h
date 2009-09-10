// DoubleExpSynMech.h: interface for the Synapse class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ALPHASYNMECH_H
#define ALPHASYNMECH_H

#include <math.h>

#include "SimulationEnvironment.h"
#include "ConfigBase.h"
#include "Error.h"
#include "GlobalDefs.h"
#include "InterfaceBase.h"


class DoubleExpSynMech;


///////////////////////////////////////////////////////////////////////////////////////////////////
// DoubleExpSynMechConfig (conductance synapse configurator) class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

class DoubleExpSynMechConfig
        : public ConfigBase
{
public:
    //! Type related to this configurator type.
    /*! Used for automated construction of neurons from configurators.
        See NeuronFactory and SynapseFactory.
     */
    typedef DoubleExpSynMech related_component;

    //! Accept method for visitor (see class template Visitor).
    MAKE_VISITABLE(DoubleExpSynMechConfig)

public:
//  DoubleExpSynMechConfig(double tauA, double tauB);
    DoubleExpSynMechConfig(std::ifstream & is);
    inline const double & factorA() const {
        return factorA_;
    }
    inline const double & factorB() const {
        return factorB_;
    }
    inline const double & renorm() const {
        return renorm_;
    }
private:
    double factorA_, factorB_, renorm_;
#ifdef _DEBUG
    double tauA_, tauB_; // only available in DEBUG mode, just for information anyway
#endif
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// DoubleExpSynMech definition
// models a synapse with a double exponential synaptic current injection from each event
///////////////////////////////////////////////////////////////////////////////////////////////////

class DoubleExpSynMech
        : public SynMechInterface
{
    template <class SynData, class SynMech, class PlastMech, class TypeBase> friend class SynapseTemplate;
    template <class PoissonParameter, class ActivationFunction> friend class PoissonMech;
public:
    explicit DoubleExpSynMech(ConfigBase * const configurator);
    ~DoubleExpSynMech() {
        delete[] alphajump_;
    }
protected:
//  template <class TypeImpl> void send_updated_states(TypeImpl & neuron);
    void send_updated_states(double & current);
    void send_updated_states(double & conductance, double & current) {
        throw Error("DoubleExpSynMech: should not be used");
    }
    void on_preneuron_fire_update(const Time & delayedspike_time, const double & synweight);
private:
    void perform_calculations();
    const DoubleExpSynMechConfig * const asmcfg_;
    double current_; // synaptic influx
    double alphaA_, alphaB_; // state variables for conductance
    double * alphajump_;
    Size nextjump_; //
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// constructor
inline DoubleExpSynMech::DoubleExpSynMech(ConfigBase * const configurator)
        : asmcfg_(dynamic_cast<DoubleExpSynMechConfig *>(configurator))
        , current_(0.)
        , alphaA_(0.)
        , alphaB_(0.)
        , nextjump_(0)
{
    if (! asmcfg_) throw ConfigError("DoubleExpSynMech: void configurator");

    alphajump_ = new double [ SimEnv::i_max_delay()];
    for (Size i = 0; i < SimEnv::i_max_delay(); ++i)
        alphajump_[i] = 0.0;
}

/////////////////////////////////////////////////
// update when the presynaptic neuron fires
inline void DoubleExpSynMech::on_preneuron_fire_update(const Time & time_lag_to_spike, const double & synweight)
{
    Size spot = nextjump_ + (Size) ceil(time_lag_to_spike / SimEnv::timestep());
    if (spot >= SimEnv::i_max_delay()) spot -= SimEnv::i_max_delay();
    alphajump_[spot] += synweight;
}

/////////////////////////////////////////////////
// send the updated states (current) to the neuron activation mechanism
inline void DoubleExpSynMech::send_updated_states(double & current)
{
    perform_calculations();
    current += current_;
}

/////////////////////////////////////////////////
// update when the presynaptic neuron fires
inline void DoubleExpSynMech::perform_calculations()
{
    ++nextjump_;
    if (nextjump_ == SimEnv::i_max_delay()) nextjump_ = 0;

    alphaA_ *= asmcfg_->factorA();
    alphaA_ += alphajump_[nextjump_];
    alphaB_ *= asmcfg_->factorB();
    alphaB_ += alphajump_[nextjump_];

    current_ = (alphaB_ - alphaA_) * asmcfg_->renorm();

    alphajump_ [nextjump_] = 0.0;
}


#endif // !defined(ALPHASYNMECH_H)
