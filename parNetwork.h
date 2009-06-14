// Network.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PARNETWORK_H
#define PARNETWORK_H

#include <list>
#include <map>

#include "Group.h"
#include "GlobalDefs.h"


#include "ParSpike.h"
#include "NetPar.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Network class definition
// to handle a collection groups of neurons, create them from a configuration file, the outputs, etc.
///////////////////////////////////////////////////////////////////////////////////////////////////



class ParNetwork 
{
public:
	ParNetwork() {}//par_ = new ParSpike;}
	//ParNetwork(ParSpike * par) { par_ = par;)
	~ParNetwork();
	//~ParNetwork(){if (par_) delete par_;}
	void build_from_file(std::string filename, std::string logfilename = "log.dat", bool no_output = false);
	void update();
	void clear_past_of_spike_list(const Time & time_end_past);
	void build_network();
//	ParSpike * par_;
	typedef	std::map< int, SynMechInterface> Gid2PreSyn;  //Similar to NEURON's hash table for parallel program
	static Gid2PreSyn* gid2out_;
	static Gid2PreSyn* gid2in_;

	int network_size();
	//void psl_append(PreSynPtr p){ presyn_list.push_back(p);}
protected:
    	typedef std::list<boost::shared_ptr<PreSyn> > ListSynType;//SynMechInterface
	typedef std::list<boost::shared_ptr<NeuronInterface> > ListNrnType;
	typedef std::list<boost::shared_ptr<Group> > ListGroupType;
   	//typedef std::map<int, boost::shared_ptr<ConfigBase> > MapConfigType;
	typedef std::list<boost::shared_ptr<ConfigBase> > ListConfigType;
	ListGroupType gp_list_; // list of pointers to the groups of the network
	ListConfigType cfg_list_; // list of pointers to the configurators for the connections to keep
//	const DataCommonNeuron::ListSynMechType & DataCommonNeuron


//	typedef	std::map<int,boost::shared_ptr<ConfigBase>> MapCellId;  //Cell id map

	ListNrnType cell_list;  // list of pointers to all the neurons of the network
	ListSynType presyn_list,postsyn_list;
//	MapCellId2Presyn gid2presyn;


};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Network inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Update method
// all the groups are updates from #0 to last one
inline void ParNetwork::update()
{
	for (ListGroupType::const_iterator i = gp_list_.begin(); 
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


///////////////////////////////////////////////////////////////////////////
// Method to 
// all the groups are updates from #0 to last one
inline void ParNetwork::build_network()
{
	for (ListGroupType::const_iterator i = gp_list_.begin(); 
		 i != gp_list_.end(); 
		 ++i)
		for (ListNrnType::iterator j = (*i)->list_.begin(); 
		 j != (*i)->list_.end(); 
		 ++j)
		cell_list.push_back((*j));

}

inline int ParNetwork::network_size()
{
	return cell_list.size();
}



#endif // !defined(NETWORK_H)
