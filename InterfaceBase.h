// InterfaceBase.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef INTERFACEBASE_H
#define INTERFACEBASE_H

#include <deque>
#include <boost/noncopyable.hpp>

#include "Visitor.h"
#include "GlobalDefs.h"
#include "IdCounter.h"



template <class T>
struct Bide
{
    typedef T X;
};


class NeuronInterface;

///////////////////////////////////////////////////////////////////////////////////////////////////
// LinkSynNrnInterfaceBase class definition
// general interface
// the method performcaculations() is to be called by the neuron within update()
///////////////////////////////////////////////////////////////////////////////////////////////////

class SynMechInterface
//    : public boost::noncopyable
{
	template <class PoissonParameter, class ActivationFunction> friend class PoissonMech;
	friend class IFMech;
	friend class InstantIFMech;
	friend class DataCommonNeuron;
public:
	virtual ~SynMechInterface() {}
	double weight;
	double delay;
protected:
	virtual void send_updated_states(double & current) = 0;
	virtual void send_updated_states(double & conductance, double & current) = 0;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// SynapseInterface class definition
// general interface
// the method on_preneuron_fire_update() is to be called by the preneuron when it fires, within notify_firing_impl() of the DataCommonNeuron class
// the methods on_preneuron_fire_plast_update() and on_postneuron_fire_plast_update() are to be called by the neurons within plast_update()
///////////////////////////////////////////////////////////////////////////////////////////////////

class SynapseInterface
{
	friend class DataPlastNeuron;
	friend class DataCommonNeuron;
public:
	SynapseInterface(NeuronInterface * post_nrn) : post_nrn_(post_nrn) {}
	virtual ~SynapseInterface() {}
	virtual const double & weight() = 0;
	const NeuronInterface & post_nrn() const {return *post_nrn_;}
#ifdef PARALLELSIM
	void send(double sendtime, ConfigBase*);
	void deliver(double, ConfigBase*);
	void record(double t);
	int gid_;
	unsigned char localgid_; // compressed gid for spike transfer
	std::deque<double> tqe_; //stores delivered events

#endif
protected:
	virtual void on_preneuron_fire_update(const double & spike_time) = 0;
	virtual void on_preneuron_fire_plast_update(const double & spike_time) = 0;
	virtual void on_postneuron_fire_plast_update(const double & spike_time) = 0;
	virtual SynMechInterface * const syn_mech() const = 0;
private:
	NeuronInterface * const post_nrn_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// NeuronInterface class definition
// general interface
// the method add_presynapse() is to be called in the main, to connect the network
// the method update() is to be called in the main, to run the simulation at each time step increase
// the method plast_update() is to be called in the main, after the update of all the neurons, to 
///////////////////////////////////////////////////////////////////////////////////////////////////

class NeuronInterface
	: public IdCounter<NeuronInterface>
{
	template <class SynData, class SynMech, class PlastMech, class TypeBase> friend class SynapseTemplate;
public:
	NeuronInterface() {}
	virtual ~NeuronInterface() {}
	virtual const Volt & potential() const = 0;
	virtual void apply_visitor(AbstractVisitor & vis) = 0;
	virtual void update() = 0; // XXX passer en protected
	// bool operator==( const std::list<NeuronInterface*>::const_iterator& itRHS)
	//{return(itLHS == itRHS);} 	
	//bool operator!=(const std::list<NeuronInterface*>::const_iterator& itRHS)
	//{return(!(this->itLHS == itRHS));} 
#ifdef PARALLELSIM
	void set_gid(int id) { gid_ = id;}
	const int gid(){ return gid_;}
protected: 
	int gid_;
#endif

protected:
	//std::list<NeuronInterface*>::iterator itLHS;
	virtual void add_presynapse(SynapseInterface * const syn) = 0;
	virtual void add_postsynapse(SynapseInterface * const syn) = 0;


};



#endif // !defined(INTERFACEBASE_H)
