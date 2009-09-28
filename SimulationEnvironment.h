// SimulationEnvironment.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SIMULATIONENVIRONMENT_H
#define SIMULATIONENVIRONMENT_H

#include <fstream>

#include "GlobalDefs.h"
//#include "Network.h"


//! SimEnv: simulation environment variables.
/*! This class encapsulates the environment variables (static data for fast access).
 */
class SimEnv
{
public:
    //! Sets the static variables of the environment.
    /*! \param is STL in stream (std::ifstream).
     */
    static void set(std::ifstream & is);
    //!
    static inline void advance() {
        ++i_time_; sim_time_ += timestep_;
    }

//  static inline void build_net_from_file(const std::string & file_name) {net_.build_from_file(file_name);}
//  static inline void activation_update() {net_.update();}

    static inline const Time & timestep() {
        return timestep_;
    }
    static inline const Time & tstop() {
      return timestep_*((double)i_duration_);
    }
    static inline const DiscreteTime & i_duration() {
        return i_duration_;
    }
    static inline const unsigned & i_max_delay() {
        return i_max_delay_;
    }
    static inline const DiscreteTime & i_time() {
        return i_time_;
    }
    static inline const Time & sim_time() {
        return sim_time_;
    }
    static inline const Time & plasticity_effective_start_time() {
        return plasticity_effective_start_time_;
    }

    static const long & random_init_seed() {
        return random_init_seed_;
    }
    static const bool & reinit_random_before_sim() {
        return reinit_random_before_sim_;
    }

private:
    // sim time related constants
    static Time timestep_; // time step of the simulation (in seconds)
    static DiscreteTime i_duration_; // duration of the simulation in timesteps
    static unsigned i_max_delay_; // maximum delay in timesteps
    static DiscreteTime i_time_; // current discretised time during the simulation in timesteps
    static Time sim_time_; // current discretised time during the simulation in timesteps
    static Time plasticity_effective_start_time_; // time starting when the plasiticy update applies

    // network
//  static Network net_;

    // optional features
    static long random_init_seed_; // to initialise the random number generator
    static bool reinit_random_before_sim_; // if 'true', reinit the random number generator after the construction of the network just before starting the simulation


#ifdef PARALLELSIM

#else

#endif
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////


#endif // !defined(SIMULATIONENVIRONMENT_H)
