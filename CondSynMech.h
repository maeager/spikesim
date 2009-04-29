// CondSynMech.h: interface for the CondSynMech class.
//////////////////////////////////////////////////////////////////////

#ifndef CONDSYNMECH_H
#define CONDSYNMECH_H

#include <math.h> 

#include "SimulationEnvironment.h"
#include "ConfigBase.h"
#include "Error.h"
#include "GlobalDefs.h"
#include "InterfaceBase.h"


class CondSynMech;


///////////////////////////////////////////////////////////////////////////////////////////////////
// CondSynMechConfig (conductance synapse configurator) class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

class CondSynMechConfig
	: public ConfigBase
{
public:
	//! Type related to this configurator type.
	/*!	Used for automated construction of neurons from configurators.
		See NeuronFactory and SynapseFactory.
	 */
	typedef CondSynMech related_component;

	//! Accept method for visitor (see class template Visitor).
	MAKE_VISITABLE(CondSynMechConfig)

public:
	CondSynMechConfig(std::ifstream & is);
	inline const double & factorA() const {return factorA_;}
	inline const double & factorB() const {return factorB_;}
	inline const double & Vreversal() const {return Vreversal_;}
private:
	double factorA_, factorB_, Vreversal_; // tauA and tauB just for information, not used in processing
#ifdef _DEBUG
	double tauA_, tauB_; // only available in DEBUG mode, just for information anyway
#endif
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// CondSynMech definition
// unitary synaptic conductance from event at t=0:g(t)=gbar*(exp(-t/tauA -exp(-m*t/tauB))
///////////////////////////////////////////////////////////////////////////////////////////////////

class CondSynMech
	: public SynMechInterface
{
	template <class SynData, class SynMech, class PlastMech, class TypeBase> friend class SynapseTemplate;
	friend class IFMech;
public:
	explicit CondSynMech(ConfigBase * const cfg);
	~CondSynMech() {delete[] alphajump_;}
protected:
//	template <class TypeImpl> void get_updated_states(TypeImpl & neuron)
	void send_updated_states(double & current) {throw Error("CondSynMech: should not be used");}
	void send_updated_states(double & conductance, double & current);
	void on_preneuron_fire_update(const Time & tspike, const double & synweight);
private:
	//! Updates the state of the synapse after one time step.
	/*!	.
	 */
	void perform_calculations();
	//! Configurator with the parameters of the synapse.
	const CondSynMechConfig * const csmcfg_; // tauA_, tauB_, factorA_, factorB_, gmin_, gmax_, Vreversal_
	double conductance_, current_;
	double alphaA_, alphaB_; // state variables for conductance
	double * alphajump_;
	Size nextjump_; // 
	Size spot_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// constructor
inline CondSynMech::CondSynMech(ConfigBase * const configurator)
	: csmcfg_(dynamic_cast<CondSynMechConfig *>(configurator))
	, current_(0.)
	, conductance_(0.)
	, alphaA_ (0.)
	, alphaB_(0.)
	, nextjump_(0)
{
	if (! csmcfg_) throw ConfigError("CondSynMech: void configurator");

	alphajump_ = new double [ SimEnv::i_max_delay() ];
	for (Size i = 0; i < SimEnv::i_max_delay(); ++i)
		alphajump_[i] = 0.0;
}

/////////////////////////////////////////////////
// update when the presynaptic neuron fires
inline void CondSynMech::on_preneuron_fire_update(const Time & time_lag_to_spike, const double & synweight)
{
//	old style
//	Size spot = ((Size) ceil( time_lag_to_spike / SimEnv::timestep() ) + nextjump_ ) % SimEnv::i_max_delay();
	
	spot_ = nextjump_ + (Size) ceil( time_lag_to_spike / SimEnv::timestep() );
	if ( spot_ >= SimEnv::i_max_delay() ) spot_ -= SimEnv::i_max_delay();

#ifdef _DEBUG
	if ( ( spot_ >= SimEnv::i_max_delay() ) || ( spot_ < 0 ) ) 
		std::cout << "CondSynMech: wrong spot_";
#endif

	alphajump_[spot_] += synweight;
}

/////////////////////////////////////////////////
// send the updated states (current and conductance) to the neuron activation mechanism
inline void CondSynMech::send_updated_states(double & conductance, double & current)
{
	perform_calculations();
	current += current_;
	conductance += conductance_;
}

/////////////////////////////////////////////////
// update when the presynaptic neuron fires
inline void CondSynMech::perform_calculations()
{
	++nextjump_;
	if ( nextjump_ == SimEnv::i_max_delay() ) nextjump_ = 0;

	alphaA_ *= csmcfg_->factorA();
	alphaA_ += alphajump_[nextjump_];
	alphaB_ *= csmcfg_->factorB();
	alphaB_ += alphajump_[nextjump_];
	// prepare the results of the computation to the Synapse object
	conductance_ = alphaB_ - alphaA_;
	current_ = conductance_ * csmcfg_->Vreversal();

	alphajump_ [nextjump_] = 0.0;
}



#endif // !defined(CONDSYNMECH_H)
