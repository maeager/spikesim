// InstantSynMech.h: interface for the CondSynMech class.
//////////////////////////////////////////////////////////////////////

#ifndef INSTANTSYNMECH_H
#define INSTANTSYNMECH_H

#include <math.h>

#include "SimulationEnvironment.h"
#include "ConfigBase.h"
#include "Error.h"
#include "GlobalDefs.h"
#include "InterfaceBase.h"
#include "RandomGenerator.h"


class InstantSynMech;

///////////////////////////////////////////////////////////////////////////////////////////////////
// InstantSynMechConfig (instantaneous synapse configurator) class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

class InstantSynMechConfig
        : public ConfigBase
{
public:
    //! Type related to this configurator type.
    /*! Used for automated construction of neurons from configurators.
        See NeuronFactory and SynapseFactory.
     */
    typedef InstantSynMech related_component;

    //! Accept method for visitor (see class template Visitor).
    MAKE_VISITABLE(InstantSynMechConfig)

    double poisson_increment() const; // Poisson background input
public:
//  InstantSynMechConfig(double Vreversal, double BkgInpRate, double BkgInpWeight);
    InstantSynMechConfig(std::ifstream & is);
    inline const double & Vreversal() const {
        return Vreversal_;
    }
    ~InstantSynMechConfig() {}
private:
    double Vreversal_;
    // for Poisson distributed background input:
    double BkgInpRate_;
    double BkgInpWeight_;
    void MakeCondProbTable();
    static const int CP_size = 1000;
    double CondProb[CP_size];
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// InstantSynMech definition
// ???
///////////////////////////////////////////////////////////////////////////////////////////////////

class InstantSynMech
        : public SynMechInterface
{
    template <class SynData, class SynMech, class PlastMech, class TypeBase> friend class SynapseTemplate;
    friend class InstantIFMech;
public:
    explicit InstantSynMech(ConfigBase * const cfg);
    ~InstantSynMech() {
        delete[] alphajump_;
    }
protected:
    void send_updated_states(double & current) {
        throw Error("InstantSynMech: should not be used");
    }
    void send_updated_states(double & conductance, double & current);
    void on_preneuron_fire_update(const Time & tspike, const double & synweight);
private:
    void perform_calculations();
    const InstantSynMechConfig * const ismcfg_;
    /*add the array alpha_, nextjump_, etc.*/
    double conductance_, current_;
    double * alphajump_;
    Size nextjump_; //
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// constructor
inline InstantSynMech::InstantSynMech(ConfigBase * const configurator)
        : ismcfg_(dynamic_cast<InstantSynMechConfig *>(configurator))
        , current_(0.)
        , conductance_(0.)
        , nextjump_(0)
{
    if (! ismcfg_) throw ConfigError("InstantSynMech: void configurator");

    alphajump_ = new double [ SimEnv::i_max_delay()];
    for (Size i = 0; i < SimEnv::i_max_delay(); ++i)
        alphajump_[i] = 0.0;
}

/////////////////////////////////////////////////
// update when the presynaptic neuron fires
inline void InstantSynMech::on_preneuron_fire_update(const Time & time_lag_to_spike, const double & synweight)
{
    Size spot = nextjump_ + (Size) ceil(time_lag_to_spike / SimEnv::timestep());
    if (spot >= SimEnv::i_max_delay()) spot -= SimEnv::i_max_delay();
    alphajump_[spot] += synweight;
}

/////////////////////////////////////////////////
// send the updated states (current and conductance) to the neuron activation mechanism
// to send states to IF mechanism
inline void InstantSynMech::send_updated_states(double & conductance, double & current)
{
    perform_calculations();
    current += current_;
    conductance += conductance_;
}

/////////////////////////////////////////////////
// update when the presynaptic neuron fires
inline void InstantSynMech::perform_calculations()
{
    ++nextjump_;
    if (nextjump_ == SimEnv::i_max_delay()) nextjump_ = 0;

    // prepare the results of the computation to the Synapse object
    // note the inclusion of background input:
    conductance_ = alphajump_[nextjump_] + ismcfg_->poisson_increment();

    current_ = conductance_ * ismcfg_->Vreversal();

    alphajump_ [nextjump_] = 0.0;
}


#endif // !defined(INSTANTSYNMECH_H)
