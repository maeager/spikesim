// DeltaCorrInputRefs.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DELTACORRINPUTREFS_H
#define DELTACORRINPUTREFS_H

#include <deque>

#include "CorrInputMech.h"
#include "SimulationEnvironment.h"
#include "RandomGenerator.h"
#include "ManageableInput.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// DeltaCorrInputRef class definition
// serves as common reference
///////////////////////////////////////////////////////////////////////////////////////////////////

class DeltaCorrInputRef
	: public ConfigBase
	, public ManageableInput
{
	friend std::ofstream & operator<<(std::ofstream & os, const DeltaCorrInputRef & cir);
public:
	//! Type related to this configurator type.
	/*!	Used for automated construction of neurons from configurators.
		See NeuronFactory and SynapseFactory.
	 */
	typedef CorrInputMech<DeltaCorrInputRef> related_component;

	//! Accept method for visitor (see class template Visitor).
	MAKE_VISITABLE(DeltaCorrInputRef)

public:
//	DeltaCorrInputRef(double base_rate, double correlation);
	DeltaCorrInputRef(std::ifstream & is);
	inline const double & rate_when_not_correlated() const {return rate_when_not_correlated_;}
	inline const double & rate_when_correlated() const {return rate_when_correlated_;}
	const bool & correlated_event_state() const;
private:
	void input_update();
	double base_rate_;
	double correlation_;
	double rate_when_not_correlated_, rate_when_correlated_;
	bool correlated_event_state_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// CorrInputRefShiftedCopy class definition
// serves as common reference
///////////////////////////////////////////////////////////////////////////////////////////////////

class CorrInputRefShiftedCopy
	: public ConfigBase
	, public ManageableInput
{
	friend std::ofstream & operator<<(std::ofstream & os, const CorrInputRefShiftedCopy & cirsc);
public:
	//! Type related to this configurator type.
	/*!	Used for automated construction of neurons from configurators.
		See NeuronFactory and SynapseFactory.
	 */
	typedef CorrInputMech<CorrInputRefShiftedCopy> related_component;

	//! Accept method for visitor (see class template Visitor).
	MAKE_VISITABLE(CorrInputRefShiftedCopy)

public:
//	CorrInputRefShiftedCopy(DeltaCorrInputRef * reference, unsigned delay_in_timestep);
	CorrInputRefShiftedCopy(std::ifstream & is);
	~CorrInputRefShiftedCopy();
	inline const double & rate_when_not_correlated() const {return reference_->rate_when_not_correlated();}
	inline const double & rate_when_correlated() const {return reference_->rate_when_correlated();}
	inline const unsigned & delay_in_timestep() const {return delay_in_timestep_;}
	const bool & correlated_event_state() const;
private:
	void input_update();
	DeltaCorrInputRef * reference_;
	Size delay_in_timestep_, index_;
	bool * correlated_event_state_list_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// updates the correlation state of a correlated group neuron
inline void DeltaCorrInputRef::input_update()
{
	if (RandomGenerator::probability_trial_for_time_step(base_rate_)) // same rate as base rate
		correlated_event_state_ = true;
	else
		correlated_event_state_ = false;
}

/////////////////////////////////////////////////////////////
// updates the correlation state of a correlated group neuron
inline const bool & DeltaCorrInputRef::correlated_event_state() const
{
	return correlated_event_state_;
}


/////////////////////////////////////////////////////////////
// updates the correlation state of a correlated group neuron
inline void CorrInputRefShiftedCopy::input_update()
{
	// retrieve the state of the reference
	correlated_event_state_list_[index_] = reference_->correlated_event_state();
	// move the index forward (and go back beginning of cycle if needed)
	++index_;
	if (index_ > delay_in_timestep_) index_ = 0;
}

/////////////////////////////////////////////////////////////
// updates the correlation state of a correlated group neuron
inline const bool & CorrInputRefShiftedCopy::correlated_event_state() const
{
	return correlated_event_state_list_[index_];
}




#endif // !defined(DELTACORRINPUTREFS_H
