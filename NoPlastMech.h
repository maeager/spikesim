// NoPlastMech.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NOPLASTMECH_H
#define NOPLASTMECH_H


class NoPlastMech;



//! NoPlastMechConfig:
/*!
 */
class NoPlastMechConfig
        : public ConfigBase
{
public:
    //! Type related to this configurator type.
    /*! Used for automated construction of neurons from configurators.
        See NeuronFactory and SynapseFactory.
     */
    typedef NoPlastMech related_component;

    //! Accept method for visitor (see class template Visitor).
    MAKE_VISITABLE(NoPlastMechConfig)
};



//! NoPlastMech:
/*!
 */
class NoPlastMech
{
protected:
    explicit NoPlastMech(ConfigBase * configurator) {
        if (! dynamic_cast<NoPlastMechConfig *>(configurator))
            throw ConfigError("NoPlastMech: void configurator");
    }
    template <class TypeImpl> void on_preneuron_fire_plast_update(TypeImpl & neuron, const Time & pre_spike_time) {}
    template <class TypeImpl> void on_postneuron_fire_plast_update(TypeImpl & neuron, const Time & post_spike_time) {}
};



#endif // !defined(NOPLASTMECH_H)
