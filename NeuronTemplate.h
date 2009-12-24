// NeuronTemplate.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NEURONTEMPLATE_H
#define NEURONTEMPLATE_H


#include "ConfigBase.h"
#include "Visitor.h"



//! NeuronTemplate class template definition
/*! data (list of synapses, etc) are in NrnData
 *the activation update mechanism is dealt with by NrnActMech
 *the interface is TypeBase
 */
template < class NrnData
, class NrnActMech
, class TypeBase >
class NeuronTemplate
        : public NrnData
        , public NrnActMech
        , public TypeBase
{
    friend class Bide<NrnActMech>::X;
public:
    NeuronTemplate(ConfigBase * datacfg, ConfigBase * actmechcfg);
    const Volt & potential() const {
        return NrnActMech::potential_impl();
    }
    inline void apply_visitor(AbstractVisitor & vis) {
        NrnData::apply_vis_impl(vis);
    }
protected:
    inline void update() {
        NrnActMech::perform_calculations(*this);
    }
    void add_presynapse(SynapseInterface * const syn) {
        NrnData::add_presynapse_impl(syn);
    }
    void add_postsynapse(SynapseInterface * const syn) {
        NrnData::add_postsynapse_impl(syn);
    }
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definition
///////////////////////////////////////////////////////////////////////////////////////////////////

//! constructor
template <class NrnData, class NrnActMech, class TypeBase>
NeuronTemplate<NrnData, NrnActMech, TypeBase>::NeuronTemplate(
    ConfigBase * datacfg,
    ConfigBase * actmechcfg)
        : NrnData(datacfg)
        , NrnActMech(actmechcfg)
{
}



#endif // !defined(NEURONTEMPLATE_H)
