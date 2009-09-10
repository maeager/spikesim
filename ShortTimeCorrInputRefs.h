// ShortTimeCorrInputRefs.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SHORTTIMECORRINPUTREFS_H
#define SHORTTIMECORRINPUTREFS_H

#include <deque>

#include "CorrInputMech.h"
#include "SimulationEnvironment.h"
#include "RandomGenerator.h"
#include "ManageableInput.h"


//! ShortTimeCorrInputRef:
/*! like DeltaCorrInputRef but one event impacts the next timestep as well
 */
class ShortTimeCorrInputRef
        : public ConfigBase
        , public ManageableInput
{
    friend std::ofstream & operator<<(std::ofstream & os, const ShortTimeCorrInputRef & cir);
public:
    //! Type related to this configurator type.
    /*! Used for automated construction of neurons from configurators.
        See NeuronFactory and SynapseFactory.
     */
    typedef CorrInputMech<ShortTimeCorrInputRef> related_component;

    //! Accept method for visitor (see class template Visitor).
    MAKE_VISITABLE(ShortTimeCorrInputRef)

public:
//  ShortTimeCorrInputRef(double base_rate, double correlation);
    ShortTimeCorrInputRef(std::ifstream & is);
    ~ShortTimeCorrInputRef();
    inline const double & rate_when_not_correlated() const {
        return rate_when_not_correlated_;
    }
    inline const double & rate_when_correlated() const {
        return rate_when_correlated_;
    }
    const bool & correlated_event_state() const;
private:
    void input_update();
    double base_rate_;
    double correlation_;
    double rate_when_not_correlated_, rate_when_correlated_;
    bool * correlated_event_state_list_;
    Size index_, flying_index_;
    Size time_spread_in_time_step_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////
// updates the correlation state of a correlated group neuron
inline void ShortTimeCorrInputRef::input_update()
{
    correlated_event_state_list_[index_] = false;
    // move the index forward (and go back beginning of cycle if needed)
    ++index_;
    if (index_ == time_spread_in_time_step_) index_ = 0;
    // update the array of event with test
    if (RandomGenerator::probability_trial_for_time_step(base_rate_)) {// same rate as base rate
        flying_index_ = index_;
        for (Size i = 0; i < time_spread_in_time_step_; ++i) {
            ++flying_index_;
            if (flying_index_ == time_spread_in_time_step_) flying_index_ = 0;
            correlated_event_state_list_[flying_index_] = true;
        }
    }
}

/////////////////////////////////////////////////////////////
// updates the correlation state of a correlated group neuron
inline const bool & ShortTimeCorrInputRef::correlated_event_state() const
{
    return correlated_event_state_list_[index_];
}



#endif // !defined(SHORTTIMECORRINPUTREFS_H
