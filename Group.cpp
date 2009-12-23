// Group.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "RandomGenerator.h"
#include "Group.h"
#include "TypeDefs.h"
#include "SynapseFactory.h"
#include "NeuronFactory.h"
#include "OutputManager.h"
#include "AsciiFileWrapper.h"

#ifdef PARALLELSIM
#include "ParallelNetManager.h"
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
// Group function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// Group populator
// parses the input stream (std::ifstream is) to build the right neuron configurators and calls the factory
void Group::populate(std::ifstream & is)
{
    std::string test;

    // read the number of neurons, and the optional recordings of activity

    READ_FROM_FILE(is, n, "n", "Group")

    // read data configurator
    if (is.eof())
        throw ConfigError("Group: expected type of neuron data");
    is >> test;
    if (test == "LIGHT_NRN") {
        data_cfg_ = new DataCommonNeuronConfig();
    } else if (test == "RECORD_NRN") {
        data_cfg_ = new DataRecordNeuronConfig();
    } else if (test == "PLAST_NRN") {
        data_cfg_ = new DataPlastNeuronConfig();
    } else throw ConfigError("Group: unknown type of neuron data, got '" + test + "'");

    // read activation mechanism configurator
    if (is.eof())
        throw ConfigError("Group: expected type of activation mechanism");
    is >> test;
    if (test == "LINEAR_POISSON_MECH_CFG") {
        nrn_act_cfg_ = new PoissonMechConfig<ConstantPoissonParameter, FuncIdentity>(is);
    } else if (test == "SIGMOID_POISSON_MECH_CFG") {
        nrn_act_cfg_ = new PoissonMechConfig<ConstantPoissonParameter, FuncSigmoid>(is);
    } else if (test == "OSCILLATORY_LINEAR_POISSON_MECH_CFG") {
        nrn_act_cfg_ = new PoissonMechConfig<OscillatoryPoissonParameter, FuncIdentity>(is);
    } else if (test == "OSCILLATORY_SIGMOID_POISSON_MECH_CFG") {
        nrn_act_cfg_ = new PoissonMechConfig<OscillatoryPoissonParameter, FuncSigmoid>(is);
    } else if (test == "DELTA_CORR_INPUT_REF") {
        nrn_act_cfg_ = new DeltaCorrInputRef(is);
    } else if (test == "SHORT_CORR_INPUT_REF") {
        nrn_act_cfg_ = new ShortTimeCorrInputRef(is);
    } else if (test == "CORR_INPUT_SHIFTED_COPY") {
        nrn_act_cfg_ = new CorrInputRefShiftedCopy(is);
    } else if (test == "IF_MECH_CFG") {
        nrn_act_cfg_ = new IFMechConfig(is);
    } else if (test == "INSTANT_IF_MECH_CFG") {
        nrn_act_cfg_ = new InstantIFMechConfig(is);
    } else throw ConfigError("Group: unknown neural activation mechanism, got '" + test + "'");

    //Create Neurons
    if ((! nrn_act_cfg_) || (! data_cfg_))
        throw ConfigError("Group: void group or neuron configurator");
    else {
        NeuronFactory nrnfactory(data_cfg_, nrn_act_cfg_);
        for (Size i = 0; i < n; ++i)
            list_.push_back(boost::shared_ptr<NeuronInterface>(nrnfactory.create()));
    }

    // outputs
    if (is.eof())
        throw ConfigError("Group: unexpected end of file, expected END_CREATE_GROUP");
    is >> test;
    while (test != "END_CREATE_GROUP") {
        if (test == "OUTPUT_KEY") {
            std::string key;
            Size outputter_id;
            if (is.eof())
                throw ConfigError("Group: unexpected end of file, expected two values of the OUTPUT_KEY tag");
            is >> key;
            if (is.eof())
                throw ConfigError("Group: unexpected end of file, expected two values of the OUTPUT_KEY tag");
            is >> outputter_id;
            if (key == "group")
                OutputManager::add_group_to_outputter(outputter_id, this);
            else
                throw ConfigError("Group: after the OUTPUT_KEY tag, 2nd argument (key) must be 'group' or a list of neurons [1,2,...]");
        } else throw ConfigError("Group: expected optional outputting information or 'END_CREATE_GROUP', got '" + test + "'");

        if (is.eof())
            throw ConfigError("Group: unexpected end of file, expected END_CREATE_GROUP");
        else
            is >> test;
    }
}

/////////////////////////////////////////////////
// Group destructor
Group::~Group()
{
}

/////////////////////////////////////////////////
// connect to another group with STDP synapses
void Group::connect_to(Group & targetgroup
                       , DistributionManager * const weight_distrib_cfg
                       , DistributionManager * const delay_distrib_cfg
                       , ConfigBase * const syn_mech_cfg
                       , ConfigBase * const plast_mech_cfg
                       , ConnectivityManager * const connectivity_mgr
                       , std::list<boost::shared_ptr<ConfigBase> > & cfg_list
                       , Size & nb_con)
{
    // creation of the synapse factory that will combine suitably the mechanisms (synaptic activation, plasticity, etc.)
    SynapseFactory synfactory(weight_distrib_cfg, delay_distrib_cfg, syn_mech_cfg, plast_mech_cfg, cfg_list);

    Size postnrn_counter = 0,
                           prenrn_counter = 0;
    std::list<NeuronInterface *> preneuron_list;
    // list of pre- and post-neurons for outputting
    std::list<Size> list_nb_pre_nrn, list_nb_post_nrn;
    // 'i' is an iterator on the target group (postneuron)
    for (ListNrnType::iterator i = targetgroup.list_.begin(); i != targetgroup.list_.end(); ++i) {
        // reset the list of neurons to connect
        preneuron_list.clear();
        // fill up the list
        prenrn_counter = 0;

        // 'j' is an iterator on the source group (preneuron)
        for (ListNrnType::const_iterator j = list_.begin(); j != list_.end(); ++j) {
            if ((*i) != (*j)) {
                if (connectivity_mgr->do_connect(prenrn_counter, postnrn_counter)) {
                    preneuron_list.push_back((*j).operator->());
                    ++nb_con; // only for output on the console screen
                    // add to output the neuron id to file
                    list_nb_pre_nrn.push_back((*j)->id());
                    list_nb_post_nrn.push_back((*i)->id());
                }
            }
            ++prenrn_counter;
        }

        // create the connections with postneuron i and the list of preneurons
        synfactory.create(preneuron_list, (*i).operator->());
        ++postnrn_counter;
    }

    // output the neuron id to file
    OutputManager::do_output_connectivity(list_nb_pre_nrn, list_nb_post_nrn, 0);
}

#ifdef PARALLELSIM
/////////////////////////////////////////////////
// Group populator
// parses the input stream (std::ifstream is) to build the right neuron configurators and calls the factory
void Group::populate_config(std::ifstream & is)
{
    std::string test;

    // read the number of neurons, and the optional recordings of activity
    READ_FROM_FILE(is, n, "n", "Group")
    std::cout << "In Group: size "<< n << std::endl;

    // read data configurator
    if (is.eof())
        throw ConfigError("Group: expected type of neuron data");
    is >> test;
    if (test == "LIGHT_NRN") {
        data_cfg_ = new DataCommonNeuronConfig();
    } else if (test == "RECORD_NRN") {
        data_cfg_ = new DataRecordNeuronConfig();
    } else if (test == "PLAST_NRN") {
        data_cfg_ = new DataPlastNeuronConfig();
    } else throw ConfigError("Group: unknown type of neuron data, got '" + test + "'");

    // read activation mechanism configurator
    if (is.eof())
        throw ConfigError("Group: expected type of activation mechanism");
    is >> test;
    if (test == "LINEAR_POISSON_MECH_CFG") {
        nrn_act_cfg_ = new PoissonMechConfig<ConstantPoissonParameter, FuncIdentity>(is);
    } else if (test == "SIGMOID_POISSON_MECH_CFG") {
        nrn_act_cfg_ = new PoissonMechConfig<ConstantPoissonParameter, FuncSigmoid>(is);
    } else if (test == "OSCILLATORY_LINEAR_POISSON_MECH_CFG") {
        nrn_act_cfg_ = new PoissonMechConfig<OscillatoryPoissonParameter, FuncIdentity>(is);
    } else if (test == "OSCILLATORY_SIGMOID_POISSON_MECH_CFG") {
        nrn_act_cfg_ = new PoissonMechConfig<OscillatoryPoissonParameter, FuncSigmoid>(is);
    } else if (test == "DELTA_CORR_INPUT_REF") {
        nrn_act_cfg_ = new DeltaCorrInputRef(is);
    } else if (test == "SHORT_CORR_INPUT_REF") {
        nrn_act_cfg_ = new ShortTimeCorrInputRef(is);
    } else if (test == "CORR_INPUT_SHIFTED_COPY") {
        nrn_act_cfg_ = new CorrInputRefShiftedCopy(is);
    } else if (test == "IF_MECH_CFG") {
        nrn_act_cfg_ = new IFMechConfig(is);
    } else if (test == "INSTANT_IF_MECH_CFG") {
        nrn_act_cfg_ = new InstantIFMechConfig(is);
    } else throw ConfigError("Group: unknown neural activation mechanism, got '" + test + "'");

    //Check Configs
    if ((! nrn_act_cfg_) || (! data_cfg_))
        throw ConfigError("Group: void group or neuron configurator");

    // outputs
    if (is.eof())
        throw ConfigError("Group: unexpected end of file, expected END_CREATE_GROUP");
    is >> test;
    while (test != "END_CREATE_GROUP") {
        if (test == "OUTPUT_KEY") {
            std::string key;
            Size outputter_id;
            if (is.eof())
                throw ConfigError("Group: unexpected end of file, expected two values of the OUTPUT_KEY tag");
            is >> key;
            if (is.eof())
                throw ConfigError("Group: unexpected end of file, expected two values of the OUTPUT_KEY tag");
            is >> outputter_id;
            if (key == "group")
                OutputManager::add_group_to_outputter(outputter_id, this);
            else
                throw ConfigError("Group: after the OUTPUT_KEY tag, 2nd argument (key) must be 'group' or a list of neurons [1,2,...]");
        } else throw ConfigError("Group: expected optional outputting information or 'END_CREATE_GROUP', got '" + test + "'");

        if (is.eof())
            throw ConfigError("Group: unexpected end of file, expected END_CREATE_GROUP");
        else
            is >> test;
    }

}


/////////////////////////////////////////////////
// Group populator creator
// calls the factory
void Group::create_population()
{
    //Create Neurons
    if ((! nrn_act_cfg_) || (! data_cfg_))
        throw ConfigError("Group: void group or neuron configurator");
    else {
#ifdef DEBUG
        std::cout << "Creating cells in Group " << std::endl;
#endif
        NeuronFactory nrnfactory(data_cfg_, nrn_act_cfg_);
        for (Size i = 0; i < n; ++i)
            list_.push_back(boost::shared_ptr<NeuronInterface>(nrnfactory.create()));
    }
#ifdef DEBUG
        std::cout << "Completed creatings cells in Group: size = " << list_.size() << std::endl;
#endif

}

/////////////////////////////////////////////////
// connect to another group with STDP synapses
// Note: Parallel implementation
void Group::par_connect_to(ParallelNetManager* const  pnm, Group & targetgroup
                           , DistributionManager * const weight_distrib_cfg
                           , DistributionManager * const delay_distrib_cfg
                           , ConfigBase * const syn_mech_cfg
                           , ConfigBase * const plast_mech_cfg
                           , ConnectivityManager * const connectivity_mgr
                           , std::list<boost::shared_ptr<ConfigBase> > & cfg_list
                           , Size & nb_con)
{
    // creation of the synapse factory that will combine suitably the mechanisms (synaptic activation, plasticity, etc.)
  
    SynapseFactory synfactory(weight_distrib_cfg, delay_distrib_cfg, syn_mech_cfg, plast_mech_cfg, cfg_list);

    Size postnrn_counter = 0, prenrn_counter = 0;
    std::list<NeuronInterface *> preneuron_list;
    // list of pre- and post-neurons for outputting
    std::list<Size> list_nb_pre_nrn, list_nb_post_nrn;
    // 'i' is an iterator on the target group (postneuron)
    for (ListNrnType::iterator i = targetgroup.list_.begin(); i != targetgroup.list_.end(); ++i) {
        if (pnm->gid_exists((*i)->gid())) { //Is the target postneuron on this node?
            // reset the list of neurons to connect
            preneuron_list.clear();
            // fill up the list
            prenrn_counter = 0;

            // 'j' is an iterator on the source group (preneuron)
            for (ListNrnType::const_iterator j = list_.begin(); j != list_.end(); ++j) {
                if ((*i) != (*j)) {
                    if (connectivity_mgr->do_connect(prenrn_counter, postnrn_counter)) {
                        preneuron_list.push_back((*j).operator->());
                        ++nb_con; // only for output on the console screen
                        // add to output the neuron id to file
                        list_nb_pre_nrn.push_back((*j)->id());
                        list_nb_post_nrn.push_back((*i)->id());
                    }
                }
                ++prenrn_counter;
            }

            // create the connections with postneuron i and the list of preneurons
            synfactory.create(preneuron_list, (*i).operator->());
            ++postnrn_counter;
        }
    }

    // output the neuron id to file
    OutputManager::do_output_connectivity(list_nb_pre_nrn, list_nb_post_nrn, 0);
  
}


#endif

