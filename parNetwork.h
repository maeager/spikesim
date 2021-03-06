// Network.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PARNETWORK_H
#define PARNETWORK_H

#include <list>
#include <map>

#include "Group.h"
#include "GlobalDefs.h"


#ifdef CPPMPI
#include "ParSpike.2.h"
#else
#include "ParSpike.h"
#endif
#include "NetPar.h"



//! Conn class definition
/** to handle a connectivity between groups of neurons
 */
class Conn
{
    //! List Type for Groups
    typedef boost::shared_ptr<Group> GroupType;
    typedef boost::shared_ptr<ConfigBase> BaseType; /*!< Type redefinition for the list of pointers to base class. */
    typedef boost::shared_ptr<ConnectivityManager>  ConnType; /*!< Type redefinition for the list of pointers to ConnectivityManager class. */
    typedef boost::shared_ptr<DistributionManager> DistrType; /*!< Type redefinition for the list of pointers to DistributionManager class. */
public:
    Conn(GroupType gps, GroupType gpt, BaseType sm_cfg, BaseType pm_cfg, ConnType c_cfg, DistrType wd_cfg, DistrType dd_cfg) :
            gp_source(gps), gp_target(gpt), syn_mech_cfg_(sm_cfg), plast_mech_cfg_(pm_cfg), connectivity_cfg_(c_cfg),
            weight_distrib_cfg_(wd_cfg), delay_distrib_cfg_(dd_cfg) {}
    Size gid_, source_id, target_id;

    GroupType gp_source, gp_target;
    BaseType syn_mech_cfg_, plast_mech_cfg_;
    ConnType connectivity_cfg_;
    DistrType weight_distrib_cfg_, delay_distrib_cfg_;

};

//extern class Group;

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
    void build_cell_list();


    void config_from_file(std::string filename, Size & ncells, Size &ngroups, std::string logfilename = "log.dat", bool no_output = false);
    void create();
    void connect_groups();
    void create_population();

    int network_size();
    //void psl_append(PreSynPtr p){ presyn_list.push_back(p);}
    
    
    typedef std::list< boost::shared_ptr<Conn> > ListConnType;//SynMechInterface
    ListConnType conn_list_;

//protected:
   //! List type to pointers of PreSyn 
    typedef std::list< PreSynPtr > ListSynType;//SynMechInterface
   //! List type to boost shared_pointers of Neurons 
    typedef std::list<boost::shared_ptr<NeuronInterface> > ListNrnType;
    //! List Type for Groups
    typedef std::list<boost::shared_ptr<Group> > ListGroupType;
    //! List type for configurator bases
    typedef std::list<boost::shared_ptr<ConfigBase> > ListConfigType;
    ListGroupType gp_list_; // list of pointers to the groups of the network
    ListConfigType cfg_list_; // list of pointers to the configurators for the connections to keep

    //! list of pointers to all the neurons of the network
    ListNrnType cell_list_;  
    //! list of pointers to all the pre and post synapses of the network
    ListSynType presyn_list, postsyn_list;

    static bool update_by_group;

};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Network inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////
 


///////////////////////////////////////////////////////////////////////////
//! Update method -  all the groups are updates from #0 to last one
inline void ParNetwork::update()
{
  if (update_by_group){
    for (ListGroupType::const_iterator i = gp_list_.begin();
            i != gp_list_.end();
            ++i)
        (*i)->update();
  } else {
    for (ListNrnType::const_iterator i = cell_list_.begin();
            i != cell_list_.end();
            ++i)
        (*i)->update();
  }

}

/////////////////////////////////////////////////
/// clean the spike list of the neurons up to 'time_end_past'
inline void ParNetwork::clear_past_of_spike_list(const Time & time_end_past)
{
    for (ListGroupType::const_iterator i = gp_list_.begin(); i != gp_list_.end(); ++i)
        (*i)->clear_past_of_spike_list(time_end_past);
}


// Following are reduntant - action performed by ParallelNetManager

inline void ParNetwork::build_cell_list()
{
//TODO Check this

    for (ListGroupType::const_iterator i = gp_list_.begin();
            i != gp_list_.end();
            ++i)
        for (ListNrnType::iterator j = (*i)->list_.begin();
                j != (*i)->list_.end();
                ++j)
            cell_list_.push_back((*j));

}


inline void ParNetwork::create_population(){
    for (ListGroupType::const_iterator i = gp_list_.begin();
            i != gp_list_.end();
            ++i)
        (*i)->create_population();
}

inline int ParNetwork::network_size()
{
    return cell_list_.size();
}


#endif // !defined(NETWORK_H)
