// Network.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PARNETWORK_H
#define PARNETWORK_H

#include <list>
#include <map>

#include "Group.h"
#include "GlobalDefs.h"


#include "ParSpike.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// Network class definition
// to handle a collection groups of neurons, create them from a configuration file, the outputs, etc.
///////////////////////////////////////////////////////////////////////////////////////////////////


class ParNetwork 
{
public:
	ParNetwork() {par_ = new ParSpike;}
	ParNetwork(ParSpike * par) { par_ = par;)

	~ParNetwork();
	void build_from_file(std::string filename, std::string logfilename = "log.dat", bool no_output = false);
	void update();
	void clear_past_of_spike_list(const Time & time_end_past);

	ParSpike * par_;
	static Gid2PreSyn* gid2out_;
	static Gid2PreSyn* gid2in_;


protected:
    typedef std::list<boost::shared_ptr<SynMechInterface> > ListSynType;
	typedef std::list<boost::shared_ptr<NeuronInterface> > ListNeuronType;
    typedef std::map<int, boost::shared_ptr<ConfigBase> > MapConfigType;
	typedef std::list<boost::shared_ptr<ConfigBase> > ListConfigType;
	ListGroupType gp_list_; // list of pointers to the groups of the network
	ListConfigType cfg_list_; // list of pointers to the configurators for the connections to keep
//	const DataCommonNeuron::ListSynMechType & DataCommonNeuron

	typedef	std::map< int, SynMechInterface> Gid2PreSyn;  //Similar to NEURON's hash table for parallel program
	typedef	std::map<int,boost::shared_ptr<ConfigBase>> MapCellId;  //Cell id map

	ListNeuronType cell_list;  // list of pointers to all the neurons of the network
	ListSynType presyn_list,postsyn_list;
	MapCellId2Presyn gid2presyn;


};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Network inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Update method
// all the groups are updates from #0 to last one
inline void ParNetwork::update()
{
	for (ListNrnType::const_iterator i = gp_list_.begin(); 
		 i != gp_list_.end(); 
		 ++i)
		(*i)->update();

}

/////////////////////////////////////////////////
// clean the spike list of the neurons up to 'time_end_past'
inline void ParNetwork::clear_past_of_spike_list(const Time & time_end_past)
{
	for (ListGroupType::const_iterator i = gp_list_.begin(); i != gp_list_.end(); ++i)
		(*i)->clear_past_of_spike_list(time_end_past);
}


/////////////////////////////////////////////////
// clean the spike list of the neurons up to 'time_end_past'
inline void ParNetwork::copy_nrn_lists()
{
	for (ListGroupType::const_iterator i = gp_list_.begin(); i != gp_list_.end(); ++i)
		std::copy((*i)->list_.begin(),(*i)->list_.end(),cell_list.begin());
}




#endif // !defined(NETWORK_H)
