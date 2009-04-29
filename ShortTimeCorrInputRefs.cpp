// ShortTimeCorrInputRefs.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <fstream>

#include "Macros.h"
#include "ShortTimeCorrInputRefs.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// ShortTimeCorrInputRef function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
/////////////////////////////////////////////////
// ShortTimeCorrInputRef constructor
ShortTimeCorrInputRef::ShortTimeCorrInputRef(double base_rate, double correlation, Size time_spread_in_time_step)
	: ManageableInput()
	, base_rate_(base_rate)
	, correlation_(correlation)
	, time_spread_in_time_step_(time_spread_in_time_step)
	, index_(0)
	, next_index_(1)
{
// correlation / time_spread_ ???
	if (correlation < 0) throw ConfigError("ShortTimeCorrInputRef: negative correlation value, must be positive");
	rate_when_not_correlated_ = base_rate_ * (1 - sqrt(correlation / time_spread_));
	rate_when_correlated_ = sqrt(correlation / time_spread_) / SimEnv::timestep() + base_rate_ * (1 - sqrt(correlation / time_spread_));
	
	correlated_event_state_list_ = new bool[time_spread_];
	for (unsigned i = 0; i < time_spread_; ++i) correlated_event_state_list_[i] = false;
}
*/
/////////////////////////////////////////////////
// ShortTimeCorrInputRef constructor with script file
ShortTimeCorrInputRef::ShortTimeCorrInputRef(std::ifstream & is)
	: ManageableInput()
	, index_(0)
	, flying_index_(0)
{
	std::string test;
	READ_FROM_FILE(is, base_rate_, "base_rate", "ShortTimeCorrInputRef")
	READ_FROM_FILE(is, correlation_, "correl_value", "ShortTimeCorrInputRef")
	READ_FROM_FILE(is, time_spread_in_time_step_, "time_spread_in_time_step", "ShortTimeCorrInputRef")
	if ((correlation_ < 0) || (correlation_ > 1)) throw ConfigError("ShortTimeCorrInputRef: correlation out of range [0,1]");
	if (time_spread_in_time_step_ < 0) throw ConfigError("ShortTimeCorrInputRef: time_spread_in_time_step negative");
	
	rate_when_not_correlated_ = base_rate_ * (1 - sqrt(correlation_ / time_spread_in_time_step_));
	rate_when_correlated_ = sqrt(correlation_ / time_spread_in_time_step_) / SimEnv::timestep() + base_rate_ * (1 - sqrt(correlation_ / time_spread_in_time_step_));

	correlated_event_state_list_ = new bool[time_spread_in_time_step_];
	for (unsigned i = 0; i < time_spread_in_time_step_; ++i) correlated_event_state_list_[i] = false;
}

/////////////////////////////////////////////////
// ShortTimeCorrInputRef constructor with script file
ShortTimeCorrInputRef::~ShortTimeCorrInputRef()
{
	delete[] correlated_event_state_list_;
}

/////////////////////////////////////////////////
// stream operator overloading
std::ofstream & operator<<(std::ofstream & os, const ShortTimeCorrInputRef & cir)
{
	double sqrt_correlation = (cir.rate_when_correlated() - cir.rate_when_not_correlated()) * SimEnv::timestep();
	os << "correlated_event_rate " << cir.base_rate_
		<< "; correlation " << cir.correlation_
		<< ";";
	return os;
}


