// CorrInputMech.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CORRINPUTMECH_H
#define CORRINPUTMECH_H


#include "SimulationEnvironment.h"
#include "ConfigBase.h"
#include "Error.h"
#include "SpikeTimeGenerator.h"


//! CorrInputMech: class template for correlated Poisson pulse train generators.
/*! The mechanism inside can rely on a constant rate (DeltaCorrInputRef, ShortTimeCorrInputRef) or
	oscillatory (OscillatoryDeltaCorrInputRef).
	, with constant correlation within a group, cf. common reference CorrGroupReference
 */
template <class CorrInputMechConfigType>
class CorrInputMech
{
protected:
	explicit CorrInputMech(ConfigBase * const configurator);
	template <class TypeImpl> void perform_calculations(TypeImpl & neuron);
	const Volt & potential_impl() const {return rate_;}
private:
	const CorrInputMechConfigType * const cimcfg_;
	double rate_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// constructor
template <class CorrInputMechConfigType>
CorrInputMech<CorrInputMechConfigType>::CorrInputMech(ConfigBase * const configurator)
	: cimcfg_(dynamic_cast<CorrInputMechConfigType *>(configurator))
	, rate_(0.) 
{
	if (! cimcfg_) throw ConfigError("CorrInputMech: void configurator");
}

//////////////////////////////////////////////////////////////////////
// updates the mechanism of CorrInputMech (similar to a Poisson mechanism)
template <class CorrInputMechConfigType>
template <class TypeImpl> 
void CorrInputMech<CorrInputMechConfigType>::perform_calculations(TypeImpl & neuron)
{
	if (cimcfg_->correlated_event_state())
		rate_ = cimcfg_->rate_when_correlated();
	else
		rate_ = cimcfg_->rate_when_not_correlated();

	if (RandomGenerator::probability_trial_for_time_step(rate_))  // presentrate in  Hz.
		neuron.notify_firing_impl( 0 );
//		static_cast<TypeImpl &>(*this).notify_firing_impl( SpikeTimeGenerator::gen_.generate_value() );
}

#endif // !defined(CORRINPUTMECH_H)
