// CreatedTypeMaps.h
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef CREATEDTYPEMAPS_H
#define CREATEDTYPEMAPS_H


#include "Error.h"
#include "ConfigBase.h"

#include "InterfaceBase.h"

//#include <boost/mpl/map.hpp>
//#include <boost/mpl/pair.hpp>
//#include <boost/mpl/insert.hpp>

//! Forbidden type.
/*! Used to generate compile-time errors for non-compatible neural or synaptic mechanisms.
 */
class NullType
        : public NeuronInterface //XXX
{
    NullType() {}
public:
    NullType(ConfigBase * const, ConfigBase * const) {
        throw ConfigError("NullType: shouldnt be called");
    }
private:
    void update() {}
    void apply_visitor(AbstractVisitor & vis) {}
    void add_presynapse(SynapseInterface * const syn) {}
    void add_postsynapse(SynapseInterface * const syn) {}

};



//! type map of the policies corresponding to the construciton of the neuron classes
/*!
 */
template < class Data
, class ActMech >
struct NeuronTypeMap
{
    typedef NullType created_type;
};

//typedef boost::mpl::map<> NeuronTypeMap2;



//! SynapseTypeMap: type map of the policies corresponding to the construciton of the synapse classes
/*!
 */
template < class Data
, class SynMech
, class PlastMech >
struct SynapseTypeMap
{
    typedef NullType created_type;
};



#endif // !defined(CREATEDTYPEMAPS_H)
