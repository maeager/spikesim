// SimulationEnvironment.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////


#include <time.h>

#include "Error.h"
#include "Macros.h"
#include "SimulationEnvironment.h"
#include "SpikeTimeGenerator.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// SimEnv function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// SimEnv static members
Time SimEnv::timestep_ = 0.0001;
DiscreteTime SimEnv::i_duration_ = 1;
unsigned SimEnv::i_max_delay_ = 1;
DiscreteTime SimEnv::i_time_ = 0;
Time SimEnv::sim_time_ = 0.;
Time SimEnv::tstop_ = 1;
Time SimEnv::plasticity_effective_start_time_ = 0.;
long SimEnv::random_init_seed_ = 1;
bool SimEnv::reinit_random_before_sim_ = false;

UniformDistribution SpikeTimeGenerator::gen_(SimEnv::timestep() / 2, SimEnv::timestep() / 2);

/////////////////////////////////////////////////
// SimEnv (re)initialisation
void SimEnv::set(std::ifstream & is)
{
    std::string test;

    // simulation time step
    READ_FROM_FILE(is, timestep_, "timestep", "SimEnv")
    if (timestep_ <= 0) throw ConfigError("SimEnv: timestep must be >0");

    // simulation duration
    double duration;
    READ_FROM_FILE(is, duration, "duration", "SimEnv")
    if (duration <= 0) throw ConfigError("SimEnv: duration must be >0");
    i_duration_ = (DiscreteTime)(ceil(duration / timestep_));
	tstop_ = timestep_ *  (Time)(i_duration_); 

    // max value for the synaptic delays
    // the '+1' is incoporated
    double max_delay;
    READ_FROM_FILE(is, max_delay, "max_delay", "SimEnv")
    if (max_delay <= 0) throw ConfigError("SimEnv: max_delay must be >0");
    i_max_delay_ = (unsigned)(ceil(max_delay / timestep_)) + 1;

    // optional features
    if (is.eof())
        throw ConfigError("SimEnv: unexpected end of file, expected END_SIM_INIT");
    is >> test;
    while (test != "END_SIM_INIT") {
        if (test == "random_init_seed") {
            // initialisation of the random number generator
            // if random_init_seed is positive, the initialisation is done with the actual time ('real' randomness)
            // (random_init_seed positive is the default value)
            if (is.eof())
                throw ConfigError("SimEnv: unexpected end of file, expected the value for the tag 'random_init_seed'");
            is >> random_init_seed_;
        } else if (test == "plast_start") {
            // get the time (in seconds) starting which the plasticity update applies
            if (is.eof())
                throw ConfigError("SimEnv: unexpected end of file, expected the value for the tag 'plast_start'");
            is >> plasticity_effective_start_time_;
        } else if (test == "random_init_before_sim") {
            // reinitialisation of the random number generator just before the simulation
            reinit_random_before_sim_ = true;
        } else throw ConfigError("SimEnv: unexpected optional tag, or expected 'END_SIM_INIT', got '" + test + "'");

        if (is.eof())
            throw ConfigError("SimEnv: unexpected end of file, expected END_SIM_INIT");
        else
            is >> test;
    }

    // defaulting and checks
    // positive seeds for the random number generator correspond to "real" random: initialisation with real time
    if (random_init_seed_ > 0) random_init_seed_ = -(long) time(NULL);
    if (plasticity_effective_start_time_ < 0)
        throw ConfigError("SimEnv: the effective start time for plasticity cannot be <0");
}

