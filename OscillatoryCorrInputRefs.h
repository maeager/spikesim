// OscillatoryCorrInputRefs.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef OSCILLATORYCORRINPUTREFS_H
#define OSCILLATORYCORRINPUTREFS_H

#include <deque>

#include "CorrInputMech.h"
#include "SimulationEnvironment.h"
#include "RandomGenerator.h"
#include "ManageableInput.h"



//! OscillatoryDeltaCorrInputRef: oscillatory version
/*! serves as common reference
 */
class OscillatoryDeltaCorrInputRef
        : public ConfigBase
        , public ManageableInput
{
    friend std::ofstream & operator<<(std::ofstream & os, const OscillatoryDeltaCorrInputRef & cir);
public:
    //! Type related to this configurator type.
    /*! Used for automated construction of neurons from configurators.
        See NeuronFactory and SynapseFactory.
     */
    typedef CorrInputMech<OscillatoryDeltaCorrInputRef> related_component;

    //! Accept method for visitor (see class template Visitor).
    MAKE_VISITABLE(OscillatoryDeltaCorrInputRef)

public:
//  OscillatoryDeltaCorrInputRef(double base_rate, double oscil_freq, double oscil_amplitude, double correlation);
    OscillatoryDeltaCorrInputRef(std::ifstream & is);
    inline const double & rate_when_not_correlated() const {
        return effective_correl_rate_;
    }
    inline const double & rate_when_correlated() const {
        return effective_correl_rate_;
    }
    inline const bool & correlated_event_state() const {
        return correlated_event_state_;
    }
private:
    void input_update();
    double base_rate_;
    double oscil_amplitude_;
    double oscil_freq_;
    double correlation_;
    double actual_rate_, effective_correl_rate_; // actual variables submitted to oscillations
    bool correlated_event_state_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// updates the correlation state of a correlated group neuron
inline void OscillatoryDeltaCorrInputRef::input_update()
{
    actual_rate_ = base_rate_ + oscil_amplitude_ * sin(oscil_freq_ * SimEnv::sim_time());
    // test if correlated event
    if (RandomGenerator::probability_trial_for_time_step(actual_rate_)) {
        effective_correl_rate_ = sqrt(correlation_) / SimEnv::timestep() + actual_rate_ * (1 - sqrt(correlation_));
        correlated_event_state_ = true;
    } else {
        effective_correl_rate_ = actual_rate_ * (1 - sqrt(correlation_));
        correlated_event_state_ = false;
    }
}



#endif // !defined(OSCILLATORYCORRINPUTREFS_H
