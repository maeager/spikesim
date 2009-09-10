// STDPMech.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef STDPMECH_H
#define STDPMECH_H

#include <math.h>
#include <deque>

#include "SimulationEnvironment.h"
#include "ConfigBase.h"
#include "Error.h"
#include "GlobalDefs.h"
#include "STDPFunction.h"



template <class STDPFunction, class STDPBounds> class STDPMech;


//! STDPMechConfig: mechanism configurator for STDPMech.
/*! It encapsulates the STDP parameters via STDPFunction and STDPBounds.
 */
template <class STDPFunction, class STDPBounds>
class STDPMechConfig
        : public ConfigBase
        , public STDPFunction
        , public STDPBounds
{
public:
    //! Type related to this configurator type.
    /*! Used for automated construction of neurons from configurators.
        See NeuronFactory and SynapseFactory.
     */
    typedef STDPMech<STDPFunction, STDPBounds> related_component;

    //! Accept method for visitor (see class template Visitor).
    MAKE_VISITABLE(STDPMechConfig)

public:
//  STDPMechConfig(double w_in, double w_out, double tauP, double tauD, double cP, double cD, double cutoffP, double cutoffD, double gmin, double gmax);
    STDPMechConfig(std::ifstream & is);
    inline const double & cP() const {
        return STDPFunction::cP_;
    }
    inline const double & cD() const {
        return STDPFunction::cD_;
    }
    inline const double & tauP() const {
        return STDPFunction::tauP_;
    }
    inline const double & tauD() const {
        return STDPFunction::tauD_;
    }
    inline const double & cutoffP() const {
        return STDPFunction::cutoffP_;
    }
    inline const double & cutoffD() const {
        return STDPFunction::cutoffD_;
    }
    inline const double & w_in() const {
        return w_in_;
    }
    inline const double & w_out() const {
        return w_out_;
    }
    inline const double & gmin() const {
        return STDPBounds::min_;
    }
    inline const double & gmax() const {
        return STDPBounds::max_;
    }
    inline void weight_change(double & weight, const double & dt) const {
        weight += STDPFunction::weight_change_impl(weight, dt);
    }
    inline void check_bounds(double & weight) const {
        STDPBounds::check_bounds_impl(weight);
    }
private:
    double w_in_, w_out_;
};



//! STDPMech definition
/*! models a spike-timing dependent plasticy mechanism with a double negative-exponentional as time-window function
 */
template <class STDPFunction, class STDPBounds>
class STDPMech
{
protected:
    explicit STDPMech(ConfigBase * const configurator);
    template <class TypeImpl> void on_preneuron_fire_plast_update(TypeImpl & neuron, const Time & pre_spike_time);
    template <class TypeImpl> void on_postneuron_fire_plast_update(TypeImpl & neuron, const Time & post_spike_time);
private:
    const STDPMechConfig<STDPFunction, STDPBounds> * const stdpmcfg_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////
// STDPMechConfig constructor with script file
template <class STDPFunction, class STDPBounds>
inline STDPMechConfig<STDPFunction, STDPBounds>::STDPMechConfig(std::ifstream & is)
{
    std::string test;
    double eta;
    READ_FROM_FILE(is, eta, "eta", "STDPMechConfig")
    READ_FROM_FILE(is, w_in_, "w_in", "STDPMechConfig")
    w_in_ *= eta;
    READ_FROM_FILE(is, w_out_, "w_out", "STDPMechConfig")
    w_out_ *= eta;
    double tauP, tauD, cP, cD;
    READ_FROM_FILE(is, tauP, "tauP", "STDPMechConfig")
    READ_FROM_FILE(is, tauD, "tauD", "STDPMechConfig")
    READ_FROM_FILE(is, cP, "cP", "STDPMechConfig")
    cP *= eta;
    READ_FROM_FILE(is, cD, "cD", "STDPMechConfig")
    cD *= -eta;
    STDPFunction::set_params(tauP, tauD, cP, cD);
    double gmin, gmax;
    READ_FROM_FILE(is, gmin, "gmin", "STDPMechConfig")
    READ_FROM_FILE(is, gmax, "gmax", "STDPMechConfig")
    STDPBounds::set_params(gmin, gmax);
}

/////////////////////////////////////////////////
// stream operator overloading
template <class STDPFunction, class STDPBounds>
std::ofstream & operator<<(std::ofstream & os, const STDPMechConfig<STDPFunction, STDPBounds> & stdpmcfg)
{
    os << " tauP " << stdpmcfg.tauP()
    << "; tauD " << stdpmcfg.tauD()
    << "; cP " << stdpmcfg.cP()
    << "; cD " << stdpmcfg.cD()
    << "; cutoffP " << stdpmcfg.cutoffP()
    << "; cutoffD " << stdpmcfg.cutoffD()
    << "; gmin " << stdpmcfg.gmin()
    << "; gmax " << stdpmcfg.gmax()
    << ";";
    return os;
}


/////////////////////////////////////////////////
// STDPMech constructor
template <class STDPFunction, class STDPBounds>
inline STDPMech<STDPFunction, STDPBounds>::STDPMech(ConfigBase * const configurator)
        : stdpmcfg_(dynamic_cast<STDPMechConfig<STDPFunction, STDPBounds> *>(configurator))
{
    if (! stdpmcfg_) throw ConfigError("STDPMech: void configurator");
}

/////////////////////////////////////////////////
// to update the synaptic weight (passed as a reference) when the preneuron fired
template <class STDPFunction, class STDPBounds>
template <class TypeImpl>
inline void STDPMech<STDPFunction, STDPBounds>::on_preneuron_fire_plast_update(TypeImpl & neuron, const double & pre_spike_time)
{
    Time dt;
    double pretime = pre_spike_time;
    double & weight = neuron.weight_impl();
    std::deque<Time>::const_reverse_iterator itend = neuron.postneuron_spike_list_impl().rend();

    // weight change due to firing rate
    weight += stdpmcfg_->w_in();

    // Locate the last spike of the postneuron:
    std::deque<Time>::const_reverse_iterator posttime_itr = neuron.postneuron_spike_list_impl().rbegin();

    //  check that there IS a last post-spike:
    if (posttime_itr == itend)
        return;

    // skip the last post-spike if it's later than, or same as, pretime (can only happen once in current timestep):
//  if (*posttime_itr >= pretime)
//      ++posttime_itr; // move backwards

    pretime += neuron.delay_impl(); // add the delay to the pretime since dt=posttime-pretime is always used below

    while (posttime_itr != itend) {
        dt = *posttime_itr - pretime;
        if (dt < -stdpmcfg_->cutoffD())
            break;
        // pre-spike causes depression
        stdpmcfg_->weight_change(weight, dt); // add contribution (dt < 0)
        ++posttime_itr; // move backwards
    }

    // Bounds:
    stdpmcfg_->check_bounds(weight);
}

/////////////////////////////////////////////////
// to update the synaptic weight (passed as a reference) when the postneuron fired
template <class STDPFunction, class STDPBounds>
template <class TypeImpl>
inline void STDPMech<STDPFunction, STDPBounds>::on_postneuron_fire_plast_update(TypeImpl & neuron, const double & post_spike_time)
{
    Time dt = - stdpmcfg_->cutoffD(); // meaningless initialisation, needed for tests using 'dt' later on
    double posttime = post_spike_time;
    double & weight = neuron.weight_impl();
    std::deque<Time>::const_reverse_iterator itend = neuron.preneuron_spike_list_impl().rend();

    // weight change due to firing rate
    weight += stdpmcfg_->w_out();

    // Locate the last spike of the preneuron:
    std::deque<Time>::const_reverse_iterator pretime_itr = neuron.preneuron_spike_list_impl().rbegin();

    //  check that there IS a last pre-spike:
    if (pretime_itr == itend)
        return;

    // skip the last pre-spike if it's later than posttime (can only happen once in current timestep):
    if (*pretime_itr >= post_spike_time)
        ++pretime_itr; // move backwards

    posttime -= neuron.delay_impl(); // equivalent to adding the delay to the pretime since dt=posttime-pretime is always used below

//  while (pretime_itr != itend)
//  {
//      if (posttime - *pretime_itr > -stdpmcfg_->cutoffD()) // pre-spike too late to register any change
//          break;
//      ++pretime_itr;  // move backwards
//  }

    while (pretime_itr != itend) {
        dt = posttime - *pretime_itr;
        if (dt > stdpmcfg_->cutoffP())
            break;
        // pre-spike causes potentiation
        stdpmcfg_->weight_change(weight, dt); // add contribution
        ++pretime_itr;  // move backwards
    }

    // Bounds:
    stdpmcfg_->check_bounds(weight);
}




#endif // !defined(STDPMECH_H)
