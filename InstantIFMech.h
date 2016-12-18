// InstantIFMech.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef INSTANTIFMECH_H
#define INSTANTIFMECH_H


#include "SimulationEnvironment.h"
#include "Error.h"
#include "Macros.h"
#include "ConfigBase.h"
#include "DistributionManager.h"


class InstantIFMech;

///////////////////////////////////////////////////////////////////////////////////////////////////
// InstantIFMechConfig class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

class InstantIFMechConfig
        : public ConfigBase
{
public:
    //! Type related to this configurator type.
    /*! Used for automated construction of neurons from configurators.
        See NeuronFactory and SynapseFactory.
     */
    typedef InstantIFMech related_component;

    //! Accept method for visitor (see class template Visitor).
    MAKE_VISITABLE(InstantIFMechConfig)

public:
//  InstantIFMechConfig(double tauP, double Vth, double Vr, double Vp);
    InstantIFMechConfig(std::ifstream & is);
    inline const double & tauP() const {
        return tauP_;
    }
    inline const double & Vth() const {
        return Vth_;
    }
    inline const double & Vr() const {
        return Vr_;
    }
    inline const double & Vp() const {
        return Vp_;
    }
    inline const double & Vfactor() const {
        return Vfactor_;
    }
    DistributionManager * V_init_distrib_;
private:
    double tauP_, Vth_, Vr_, Vp_, Vfactor_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// InstantIFMech definition
// models an integrate-&-fire neuron
///////////////////////////////////////////////////////////////////////////////////////////////////

class InstantIFMech
{
protected:
    explicit InstantIFMech(ConfigBase * const configurator);
    template <class TypeImpl> void perform_calculations(TypeImpl & neuron);
    const Volt & potential_impl() const {
        return v[1];
    }
private:
    const InstantIFMechConfig * const ifmcfg_;
    double v[2];  // membrane potential at consecutive time steps (mV)
    double alpha[2], beta[2]; // Array of working variables.  See Shelly&Tao. ( s^-1, mV/s)
//  double * alphasyn, * betasyn; // Array of working variables recording k for each presynpase so this doesn't need to be recalculated.
    double k[2]; // Array of working variable. See Shelly&Tao. (mV/s)
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// constructor
inline InstantIFMech::InstantIFMech(ConfigBase * const configurator)
        : ifmcfg_(dynamic_cast<InstantIFMechConfig *>(configurator))
{
    if (! ifmcfg_) throw ConfigError("InstantIFMech: void configurator");

//  v[0] = v[1] = ifmcfg_->Vr();
    // new way to initialize v:
    do
        v[0] = v[1] = ifmcfg_->V_init_distrib_->generate_value();
    while ((v[0] < ifmcfg_->Vr()) || (v[0] > ifmcfg_->Vth()));
    // warning: this would be an infinite loop if one chose a V_init_distrib that
    // only generated values outside of (Vr,Vth).
    // should throw a ConfigError if this is the case

    alpha[0] = alpha[1] = 1.0 / ifmcfg_->tauP();
    beta[0] = beta[1] = ifmcfg_->Vp() / ifmcfg_->tauP();
    k[0] = k[1] = 0;
}

/////////////////////////////////////////////////
// mechanism called to update the state of the neuron
template <class TypeImpl>
inline void InstantIFMech::perform_calculations(TypeImpl & neuron)
{
    // Update v;
    v[0] = v[1];

    // sum all synaptic contributions (continuous or impulsive conductance and current):
    for (typename TypeImpl::ListSynMechType::const_iterator i = neuron.presynmechlist_impl().begin();
            i != neuron.presynmechlist_impl().end();
            ++i)
        (*i)->send_updated_states(alpha[1], beta[1]);

    // increment v[0] with the net impulsive current
    v[0] += beta[1] - alpha[1] * v[0];
    alpha[1] = 0.0;    // re-initialise (note, only alpha[1] and beta[1] are used in instantaneous version)
    beta[1] = 0.0;

    if (v[0] > ifmcfg_->Vth()) { // the neuron spikes!
        // spike occurs at the *beginning* of the timestep:
        v[0] = ifmcfg_->Vr();

        // effect the consequences of the spike: time until the spike from the current simulation time
        neuron.notify_firing_impl(0);
    }

    // Passive evolution from t0 to t1:
    v[1] = ifmcfg_->Vp() + ifmcfg_->Vfactor() * (v[0] - ifmcfg_->Vp());
}


#endif // !defined(INSTANTIFMECH_H)
