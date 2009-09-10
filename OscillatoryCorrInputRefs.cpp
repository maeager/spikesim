// OscillatoryCorrInputRefs.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <fstream>

#include "Macros.h"
#include "OscillatoryCorrInputRefs.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// OscillatoryDeltaCorrInputRef function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
/////////////////////////////////////////////////
// OscillatoryDeltaCorrInputRef constructor
OscillatoryDeltaCorrInputRef::OscillatoryDeltaCorrInputRef(double base_rate, double oscil_freq, double oscil_amplitude, double correlation)
    : ManageableInput()
    , base_rate_(base_rate)
    , correlation_(correlation)
    , oscil_amplitude_(oscil_amplitude)
    , oscil_freq_(oscil_freq)
    , correlated_event_state_(false)
{
    if (correlation_ < 0) throw ConfigError("OscillatoryDeltaCorrInputRef: negative correlation value, must be positive");
    if (base_rate_ < oscil_amplitude_) throw ConfigError("OscillatoryDeltaCorrInputRef: amplitude of oscillations > base rate");
}
*/
/////////////////////////////////////////////////
// OscillatoryDeltaCorrInputRef constructor with script file
OscillatoryDeltaCorrInputRef::OscillatoryDeltaCorrInputRef(std::ifstream & is)
        : ManageableInput()
        , correlated_event_state_(false)
{
    std::string test;
    READ_FROM_FILE(is, base_rate_, "base_rate", "OscillatoryDeltaCorrInputRef")
    READ_FROM_FILE(is, correlation_, "correl_value", "OscillatoryDeltaCorrInputRef")
    READ_FROM_FILE(is, oscil_amplitude_, "oscil_amplitude", "OscillatoryDeltaCorrInputRef")
    READ_FROM_FILE(is, oscil_freq_, "oscil_freq", "OscillatoryDeltaCorrInputRef")

    if (correlation_ < 0) throw ConfigError("OscillatoryDeltaCorrInputRef: negative correlation value, must be positive");
    if (base_rate_ < oscil_amplitude_) throw ConfigError("OscillatoryDeltaCorrInputRef: amplitude of oscillations > base rate");
}

/////////////////////////////////////////////////
// stream operator overloading
std::ofstream & operator<<(std::ofstream & os, const OscillatoryDeltaCorrInputRef & cir)
{
    double sqrt_correlation = (cir.rate_when_correlated() - cir.rate_when_not_correlated()) * SimEnv::timestep();
    os << "correlated_event_rate " << cir.base_rate_
    << "; oscillation amplitude " << cir.oscil_amplitude_
    << "; oscillation frequency " << cir.oscil_freq_
    << "; correlation " << cir.correlation_
    << ";";
    return os;
}

