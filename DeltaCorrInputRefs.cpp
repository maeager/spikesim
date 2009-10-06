// DeltaCorrInputRefs.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <fstream>

#include "Macros.h"
#include "DeltaCorrInputRefs.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// DeltaCorrInputRef function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
/////////////////////////////////////////////////
// DeltaCorrInputRef constructor
DeltaCorrInputRef::DeltaCorrInputRef(double base_rate, double correlation)
    : ManageableInput()
    , base_rate_(base_rate)
    , correlation_(correlation)
    , correlated_event_state_(false)
{
    if (correlation_ < 0) throw ConfigError("DeltaCorrInputRef: negative correlation value, must be positive");
    rate_when_not_correlated_ = base_rate_ * (1 - sqrt(correlation_));
    rate_when_correlated_ = sqrt(correlation_) / SimEnv::timestep() + base_rate_ * (1 - sqrt(correlation_));
}
*/
/////////////////////////////////////////////////
// DeltaCorrInputRef constructor with script file
DeltaCorrInputRef::DeltaCorrInputRef(std::ifstream & is)
        : ManageableInput()
        , correlated_event_state_(false)
{
    std::string test;
    READ_FROM_FILE(is, base_rate_, "base_rate", "DeltaCorrInputRef")
    READ_FROM_FILE(is, correlation_, "correl_value", "DeltaCorrInputRef")
    if ((correlation_ < 0) || (correlation_ > 1)) throw ConfigError("DeltaCorrInputRef: correlation out of range [0,1]");

    rate_when_not_correlated_ = base_rate_ * (1 - sqrt(correlation_));
    rate_when_correlated_ = sqrt(correlation_) / SimEnv::timestep() + base_rate_ * (1 - sqrt(correlation_));
}

/////////////////////////////////////////////////
// stream operator overloading
std::ofstream & operator<<(std::ofstream & os, const DeltaCorrInputRef & cir)
{
    os << "correlated_event_rate " << cir.base_rate_
    << "; correlation " << cir.correlation_
    << ";";
    return os;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// CorrInputRefShiftedCopy function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
/////////////////////////////////////////////////
// CorrInputRefShiftedCopy constructor
CorrInputRefShiftedCopy::CorrInputRefShiftedCopy(DeltaCorrInputRef * reference, unsigned delay_in_timestep)
    : ManageableInput(false)
    , reference_(reference)
    , delay_in_timestep_(delay_in_timestep)
    , index_(0)
{
    if (! reference_) throw ConfigError("CorrInputRefShiftedCopy: void reference pulse train (expect type DeltaCorrInputRef)");
    correlated_event_state_list_ = new bool[delay_in_timestep_ +1];
    for (unsigned i = 0; i < delay_in_timestep_ +1; ++i) correlated_event_state_list_[i] = false;
}
*/
/////////////////////////////////////////////////
// CorrInputRefShiftedCopy constructor with script file
CorrInputRefShiftedCopy::CorrInputRefShiftedCopy(std::ifstream & is)
        : ManageableInput(false)
        , index_(0)
{
    throw ConfigError("CorrInputRefShiftedCopy: not functional yet... to be refactored");

    std::string test;
    unsigned num_ref;
    READ_FROM_FILE(is, num_ref, "num_ref", "CorrInputRefShiftedCopy")
    READ_FROM_FILE(is, delay_in_timestep_, "delay_in_timestep", "CorrInputRefShiftedCopy")
//  reference_ = dynamic_cast<DeltaCorrInputRef *>(ManageableInputManager::get_primary_input(num_ref));

    if (! reference_) throw ConfigError("CorrInputRefShiftedCopy: void reference pulse train (expect type DeltaCorrInputRef)");
    correlated_event_state_list_ = new bool[delay_in_timestep_ +1];
    for (unsigned i = 0; i < delay_in_timestep_ + 1; ++i) correlated_event_state_list_[i] = false;
}

/////////////////////////////////////////////////
// CorrInputRefShiftedCopy constructor with script file
CorrInputRefShiftedCopy::~CorrInputRefShiftedCopy()
{
    delete[] correlated_event_state_list_;
}

/////////////////////////////////////////////////
// stream operator overloading
std::ofstream & operator<<(std::ofstream & os, const CorrInputRefShiftedCopy & cirsc)
{
    os << "delay_in_timestep " << cirsc.delay_in_timestep_
    << ";";
    return os;
}




