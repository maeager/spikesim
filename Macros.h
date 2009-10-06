// Macros.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MACROS_H
#define MACROS_H



///////////////////////////////////////////////////////////////////////////////////////////////////
// macro to read the file (used in the config constructor from file)
// 'test' is a string used to parse the file (IMPORTANT: it has to be defined outside)
// 'is' is the input stream of the file
// 'var' is the name of the variable (in the c++ code), and 'nameinfile' its identifient in the script file
// 'scopeerrmsg' is a string ("...") used in the error management and should correspond to the class in which the macro is used
///////////////////////////////////////////////////////////////////////////////////////////////////

#define READ_FROM_FILE(filestream, var, nameinfile, scopeerrmsg)                                                                    \
    if (filestream.eof())                                                                                                       \
        throw ConfigError(std::string(scopeerrmsg) + ": unexpected end of file, expected '" + nameinfile );                         \
    filestream >> test;                                                                                                         \
    if (test == nameinfile)                                                                                                     \
        if (filestream.eof())                                                                                                   \
            throw ConfigError(std::string(scopeerrmsg) + ": unexpected end of file, expected the value of " + nameinfile );     \
        else                                                                                                                    \
            filestream >> var;                                                                                                  \
    else                                                                                                                        \
        throw ConfigError(std::string(scopeerrmsg) + ": expected '" + nameinfile + "', got '" + test + "'");




///////////////////////////////////////////////////////////////////////////////////////////////////
// macro to automate the creation of the neuron classes
///////////////////////////////////////////////////////////////////////////////////////////////////

#define NEURON_TYPE(name, data, act_mech)                                   \
    \
    typedef NeuronTemplate<data, act_mech, NeuronInterface> name;               \
    \
    template <>                                                                 \
    struct NeuronTypeMap<data, act_mech>                                        \
    {                                                                           \
        typedef name created_type;                                              \
    };

//typedef boost::mpl::insert<NeuronTypeMap2, boost::mpl::pair<getdatapolicy, getactmechpolicy> >::type NeuronTypeMap2;




///////////////////////////////////////////////////////////////////////////////////////////////////
// macro to automate the creation of the synapse classes
///////////////////////////////////////////////////////////////////////////////////////////////////

#define SYNAPSE_TYPE(name, data, syn_mech, plasticity)                          \
    \
    typedef SynapseTemplate<data, syn_mech, plasticity, SynapseInterface> name;     \
    \
    template <> struct SynapseTypeMap<data                                          \
                , syn_mech                                      \
                , plasticity>                                   \
    {                                                                               \
        typedef name created_type;                                                  \
    };




///////////////////////////////////////////////////////////////////////////////////////////////////
// macro to make a class visitable
///////////////////////////////////////////////////////////////////////////////////////////////////

#define MAKE_VISITABLE(name)                                        \
    \
    void apply_vis(AbstractVisitor & vis)                               \
    {                                                                   \
        Visitor<name > * ptvis = dynamic_cast<Visitor<name > *>(& vis); \
        if (ptvis)                                                      \
            ptvis->visit(*this);                                        \
    }

#endif // !defined(MACROS_H)
