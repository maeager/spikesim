// Network.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NETWORK_H
#define NETWORK_H

#include <list>

#include "Group.h"
#include "GlobalDefs.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// Network class definition
// to handle a collection groups of neurons, create them from a configuration file, the outputs, etc.
///////////////////////////////////////////////////////////////////////////////////////////////////


class Network
{
public:
    Network() {}
    ~Network();
    void build_from_file(std::string filename, std::string logfilename = "log.dat", bool no_output = false);
    void update();
    void clear_past_of_spike_list(const Time & time_end_past);
protected:
    typedef std::list<boost::shared_ptr<Group> > ListGroupType;
    typedef std::list<boost::shared_ptr<ConfigBase> > ListConfigType;
    ListGroupType gp_list_; // list of pointers to the neurons of the network
    ListConfigType cfg_list_; // list of pointers to the configurators for the connections to keep

};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Network inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Update method
// all the groups are updates from #0 to last one
inline void Network::update()
{
    for (ListGroupType::const_iterator i = gp_list_.begin();
            i != gp_list_.end();
            ++i)
        (*i)->update();
}

/////////////////////////////////////////////////
// clean the spike list of the neurons up to 'time_end_past'
inline void Network::clear_past_of_spike_list(const Time & time_end_past)
{
    for (ListGroupType::const_iterator i = gp_list_.begin(); i != gp_list_.end(); ++i)
        (*i)->clear_past_of_spike_list(time_end_past);
}


#endif // !defined(NETWORK_H)
