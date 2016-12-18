// IFMech.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef IFMECH_H
#define IFMECH_H

#include <math.h>

#include "SimulationEnvironment.h"
#include "Error.h"
#include "ConfigBase.h"
#include "Macros.h"


class IFMech;


///////////////////////////////////////////////////////////////////////////////////////////////////
// IFMechConfig class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

class IFMechConfig
        : public ConfigBase
{
public:
    //! Type related to this configurator type.
    /*! Used for automated construction of neurons from configurators.
        See NeuronFactory and SynapseFactory.
     */
    typedef IFMech related_component;

    //! Accept method for visitor (see class template Visitor).
    MAKE_VISITABLE(IFMechConfig)

public:
//  IFMechConfig(double tauP, double Vth, double Vr, double Vp, double refrac_per_in_seconds);
    IFMechConfig(std::ifstream & is, bool eif);
    inline const double & deltaT() const {
        return deltaT_;
    }
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
    inline const Size & refrac_per() const {
        return refrac_per_;
    }
private:
    double tauP_
    , Vth_
    , deltaT_
    , Vr_
    , Vp_;          /*!< . */
    Size refrac_per_;   /*!< Refractory period in time steps. */
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// IFMech definition
// models an integrate-&-fire neuron
///////////////////////////////////////////////////////////////////////////////////////////////////

class IFMech
{
protected:
    explicit IFMech(ConfigBase * const configurator);
    template <class TypeImpl> void perform_calculations(TypeImpl & neuron);
    const Volt & potential_impl() const {
        return v[1];
    }
private:
    const IFMechConfig * const ifmcfg_;
    double v[2];  // membrane potential at consecutive time steps (mV)
    double alpha[2], beta[2]; // Array of working variables.  See Shelly&Tao. ( s^-1, mV/s)
//  double * alphasyn, * betasyn; // Array of working variables recording k for each presynpase so this doesn't need to be recalculated.
    double k[2]; // Array of working variable. See Shelly&Tao. (mV/s)
    Size refrac_time_left_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// constructor
inline IFMech::IFMech(ConfigBase * const configurator)
        : ifmcfg_(dynamic_cast<IFMechConfig *>(configurator))
{
    if (! ifmcfg_) throw ConfigError("IFMech: void configurator");
    v[0] = v[1] = ifmcfg_->Vr();
    alpha[0] = alpha[1] = 1.0 / ifmcfg_->tauP();
    beta[0] = beta[1] = ifmcfg_->Vp() / ifmcfg_->tauP();
    k[0] = k[1] = 0;
    refrac_time_left_ = 0;
}

/////////////////////////////////////////////////
// mechanism called to update the state of the neuron
template <class TypeImpl>
inline void IFMech::perform_calculations(TypeImpl & neuron)
{
    double time_to_spike;

    bool e = (ifmcfg_->deltaT()!=0);

    // Update v;
    v[0] = v[1];

    // test if in refractory period
    if (refrac_time_left_ > 0) {
        --refrac_time_left_;
    } else {
        // update alpha and beta
        alpha[0] = alpha[1];
        beta[0] = beta[1];
        alpha[1] = 1.0 / ifmcfg_->tauP();
        beta[1] = ifmcfg_->Vp() / ifmcfg_->tauP();

        // sum all synaptic contributions (continuous or impulsive conductance and current):
        for (typename TypeImpl::ListSynMechType::const_iterator i = neuron.presynmechlist_impl().begin();
                i != neuron.presynmechlist_impl().end();
                ++i)
            (*i)->send_updated_states(alpha[1], beta[1]);

        // calculate dv/dt at start of time step
        k[0] = -alpha[0] * v[0] + beta[0];
        if (e) {
            k[0] += alpha[0] * ifmcfg_->deltaT() * exp((v[0] - ifmcfg_->Vth()) / ifmcfg_->deltaT());
        }

        // use that to estimate new V
        v[1] = v[0] + SimEnv::timestep() * k[0];

        // calculate dv/dt at end of time step
        k[1] = -alpha[1] * v[1] + beta[1];
        if (e) {
            k[1] += alpha[1] * ifmcfg_->deltaT() * exp((v[1] - ifmcfg_->Vth()) / ifmcfg_->deltaT());
        }

        // use that to calculate new V
        v[1] = v[0] + SimEnv::timestep() * (k[0] + k[1]) / 2.0;

        if (ifmcfg_->deltaT()) {
            //////
            // We can't actually simulate the spike going to infinity
            // I just calculate the time to spike analytically from the exponential spiking current, and check if it's within this time step.
            // Pressley & Troyer check when it goes above a certain triggering threshold, then calculate the time to spike analytically from there.
            // They didn't give much detail about that, though.
            // Also they use smaller time steps for greater accuracy when close to a spike. I haven't tried this.

            // Calculate time to spike analytically
            // APPROXIMATION: synaptic and leakage currents are ignored in this step. Close to a spike, exponential term is probably much bigger than them.
            time_to_spike = ifmcfg_->tauP() * exp((ifmcfg_->Vth() - v[0]) / ifmcfg_->deltaT());

            // Check if it spikes in this time step
            if (time_to_spike < SimEnv::timestep()) { // the neuron spikes!

                // reset
                // calculate spike current using Vr
                // APPROXIMATION
                // theoretically this isn't right because the voltage we're about to reset to isn't exactly Vr
                // and because I'm ignoring synaptic contributions to conductance.
                // Spike current after reset is probably small anyway, so hopefully this is a reasonable estimate.
                double psi;
                if (e) {
                    psi = ifmcfg_->deltaT() / ifmcfg_->tauP() * exp((ifmcfg_->Vr() - ifmcfg_->Vth()) / ifmcfg_->deltaT());
                } else {
                    psi = 0;
                }
                // calculate what the next V is.
                v[0] = ifmcfg_->Vr() - time_to_spike * (beta[0] + psi + beta[1] + psi - alpha[1] * (beta[0] + psi) * SimEnv::timestep()) / 2.0;
                v[0] /= 1.0 + time_to_spike * (-alpha[0] - alpha[1] + alpha[0] * alpha[1] * SimEnv::timestep()) / 2.0;
                k[0] = -alpha[0] * v[0] + beta[0] + psi;
                k[1] = -alpha[1] * (v[0] + SimEnv::timestep() * k[0]) + beta[1] + psi;
                v[1] = v[0] + SimEnv::timestep() * (k[0] + k[1]) / 2.0;
                

                // effect the consequences of the spike:
                neuron.notify_firing_impl(time_to_spike);

                // start the refractory period
                refrac_time_left_ = ifmcfg_->refrac_per();
            }
        } else {
            if (v[1] > ifmcfg_->Vth()) { // the neuron spikes!
                // determine the exact time of the next spike:
                time_to_spike = SimEnv::timestep() * (ifmcfg_->Vth() - v[0]) / (v[1] - v[0]);
                // corrections
                if (time_to_spike < 0) {
                    time_to_spike = 0;
    #ifdef _DEBUG
                    std::cout << "IFMech: negative time_to_spike";
    #endif
                } else if (time_to_spike >= SimEnv::timestep()) {
                    time_to_spike = 0;
    #ifdef _DEBUG
                    std::cout << "IFMech: too large time_to_spike";
    #endif
                }

                v[0] = ifmcfg_->Vr() - time_to_spike * (beta[0] + beta[1] - alpha[1] * beta[0] * SimEnv::timestep()) / 2.0;
                v[0] /= 1.0 + time_to_spike * (-alpha[0] - alpha[1] + alpha[0] * alpha[1] * SimEnv::timestep()) / 2.0;
                k[0] = -alpha[0] * v[0] + beta[0];
                k[1] = -alpha[1] * (v[0] + SimEnv::timestep() * k[0]) + beta[1];
                v[1] = v[0] + SimEnv::timestep() * (k[0] + k[1]) / 2.0;

                // effect the consequences of the spike:
                neuron.notify_firing_impl(time_to_spike);

                // start the refractory period
                refrac_time_left_ = ifmcfg_->refrac_per();
            }
        }
    }
}


#endif // !defined(IFMECH_H)
