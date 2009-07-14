// ParNetwork.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "StringFormatters.h"
#include "TypeDefs.h"
#include "SimulationEnvironment.h"
#include "OutputManager.h"

#include "ParNetwork.h"

ParNetwork::
~ParNetwork()
{
//	if (par_) delete par_; 
/*	cell_list.clear();
	gf_list_.clear();
	cfg_list_clear();
	presyn_list.clear();
	postsyn_list.clear();
*/
}


/////////////////////////////////////////////////
// connect to another group according to a script file describing the connections
// this function is quite awful, but most of the object construction using the script file has been
//   encapsulated in class constructors, so that only the overall process with the main tags are read here
//   almost all the parameters are not known in this scope
//----------ME------------
// TODO: This needs to be split into a configurator and an implementor, so  one process does not decide to build everything

void ParNetwork::build_from_file(std::string configfilename, std::string logfilename, bool no_output)
{
//	std::ofstream logoutfile;
//	logoutfile.open(logfilename.c_str());
	std::ifstream config_file;
	config_file.open(configfilename.c_str());

	if ((! config_file.is_open())) // || (! logoutfile.is_open()))
		throw ConfigError("Network: script file not found or cannot create log file");
	else {
		std::string test; // use to parse the config_file
		std::string error_tag; // used to build error messages

		/////////////////////////////////////////////////
		// simulation initialisation
		if (config_file.eof()) 
			throw ConfigError("Network: expected 'SIM_INIT '(to initialise the simulation environment)");
		config_file >> test;
		if (test != "SIM_INIT") throw ConfigError("Network: expected 'SIM_INIT' (to initialise the simulation environment), got '" + test + "'");
		error_tag = " (in the SIM_INIT loop)";
		// initialise the simulation environment
		SimEnv::set(config_file);
		// check for end marker END_SIM_INIT in SimEnv::set(...)

		/////////////////////////////////////////////////
		// defines the outputs
		if (config_file.eof()) 
			throw ConfigError("Network: expected 'SET_OUTPUTS'");
		config_file >> test;
		if (test != "SET_OUTPUTS") throw ConfigError("Network: expected 'SET_OUTPUTS', got '" + test + "'");
		error_tag = " (in the SET_OUTPUTS loop)";
		// initialise the simulation environment
		OutputManager::set(config_file);

		/////////////////////////////////////////////////
		// create groups of neurons and connect them
		Size create_tag_number = 0, connect_tag_number = 0;
		Size num_gp = 0, nb_con; // for console screen outputs
		while (! config_file.eof() )
		{
			test = "";
			config_file >> test;
			if (test == "")
			{} // corresponds to an empty line in the end of the file
			else if (test == "CREATE_GROUP")
			{
				++create_tag_number;
				error_tag = std::string(" (in the #") + StringFormatter::IntToStr(create_tag_number) + " " + test + " loop)";
				// create a group of neurons
				gp_list_.push_back(boost::shared_ptr<Group>(new Group()));
				gp_list_.back()->populate(config_file);
				// check for end marker END_NRN_GROUP in Group::populate(...)

				// output on screen: write out the size of the constructed group
				if (! no_output)
				{
					std::cout << "group " << num_gp << " has " << gp_list_.back()->size() << " neurons" << std::endl;
				}
				++num_gp;
			}
// COPY_GROUP ??? clonable???
			else if (test == "CONNECT")
			{
				++connect_tag_number;
				nb_con = 0;
				error_tag = std::string(" (in the #") + StringFormatter::IntToStr(create_tag_number) + " " + test + " loop)";
				ConfigBase * syn_mech_cfg, * plast_mech_cfg;
				ConnectivityManager * connectivity_cfg;
				DistributionManager * weight_distrib_cfg, * delay_distrib_cfg;
				
				// read the two groups to be connected
				Size n_source, n_target;
				READ_FROM_FILE(config_file, n_source, "n_source", "Network" + error_tag)
				READ_FROM_FILE(config_file, n_target, "n_target", "Network" + error_tag)
				ListGroupType::iterator gp_source = gp_list_.begin();
				for (Size i = 0; i < n_source; ++i)
				{
					if (gp_source == gp_list_.end())
						throw ConfigError("Network: index of source group out of bound" + error_tag);
					++gp_source;
				}
				if (gp_source == gp_list_.end())
					throw ConfigError("Network: index of source group out of bound" + error_tag);
				ListGroupType::iterator gp_target = gp_list_.begin();
				for (Size i = 0; i < n_target; ++i)
				{
					if (gp_target == gp_list_.end())
						throw ConfigError("Network: index of target group out of bound" + error_tag);
					++gp_target;
				}
				if (gp_target == gp_list_.end())
					throw ConfigError("Network: index of target group out of bound" + error_tag);
				
				// get the weight distribution
				if (config_file.eof()) 
					throw ConfigError("Network: expected the type of weight distribution" + error_tag);
				config_file >> test;
				if (test == "WEIGHT_DELTA") {
					weight_distrib_cfg = new DeltaDistribution(config_file);
				} else if (test == "WEIGHT_UNIFORM") {
					weight_distrib_cfg = new UniformDistribution(config_file);
				} else if (test == "WEIGHT_BIMODAL") {
					weight_distrib_cfg = new BimodalDistribution(config_file);
				} else if (test == "WEIGHT_GAUSSIAN") {
					weight_distrib_cfg = new GaussianDistribution(config_file);
				} else throw ConfigError("Network: unknown type of weight distribution" + error_tag + ", got '" + test + "'");
				
				// get the delay distribution
				if (config_file.eof()) 
					throw ConfigError("Network: expected the type of delay distribution" + error_tag);
				config_file >> test;
				if (test == "DELAY_DELTA") {
					delay_distrib_cfg = new DeltaDistribution(config_file);
				} else if (test == "DELAY_UNIFORM") {
					delay_distrib_cfg = new UniformDistribution(config_file);
				} else if (test == "DELAY_BIMODAL") {
					delay_distrib_cfg = new BimodalDistribution(config_file);
				} else if (test == "DELAY_GAUSSIAN") {
					delay_distrib_cfg = new GaussianDistribution(config_file);
				} else throw ConfigError("Network: unknown type of delay distribution" + error_tag + ", got '" + test + "'");

				// get the synaptic activation mechanism
				if (config_file.eof()) 
					throw ConfigError("Network: expecting the type of plasticity mechanism" + error_tag);
				config_file >> test;
				if (test == "COND_MECH") {
					syn_mech_cfg = new CondSynMechConfig(config_file);
					cfg_list_.push_back(boost::shared_ptr<ConfigBase>(syn_mech_cfg));
				} else if (test == "INSTANT_MECH") {
					syn_mech_cfg = new InstantSynMechConfig(config_file);
					cfg_list_.push_back(boost::shared_ptr<ConfigBase>(syn_mech_cfg));
				} else if (test == "KERNEL_MECH") {
					syn_mech_cfg = new DoubleExpSynMechConfig(config_file);
					cfg_list_.push_back(boost::shared_ptr<ConfigBase>(syn_mech_cfg));
				} else throw ConfigError("Network: unknown type of synaptic activation mechanism" + error_tag + ", got '" + test + "'");
				
				// get the synaptic plasticity mechanism
				if (config_file.eof()) 
					throw ConfigError("Network: expecting the type of plasticity mechanism" + error_tag);
				config_file >> test;
				if (test == "STDP") {
					plast_mech_cfg = new STDPMechConfig<AdditiveSTDPNegExpFunction, ClipToBounds>(config_file);
					cfg_list_.push_back(boost::shared_ptr<ConfigBase>(plast_mech_cfg));
				} else if (test == "NON_PLASTIC") {
					plast_mech_cfg = new NoPlastMechConfig();
				} else throw ConfigError("Network: unknown type of plasticity mechanism" + error_tag + ", got '" + test + "'");

				// get the connectivity type
				if (config_file.eof()) 
					throw ConfigError("Network: expecting the type of connectivity" + error_tag);
				config_file >> test;
				if (test == "FULL") {
					connectivity_cfg = new FullConnectivity(config_file);
				} else if (test == "RANDOM_FULL_PRECON") {
					connectivity_cfg = new RandomFullPreConnectivity(config_file, (*gp_target)->size());
				} else if (test == "RANDOM_FIXED_PRECON") {
					connectivity_cfg = new RandomFixedPreConnectivity(config_file, (*gp_source)->size(), (*gp_target)->size());
				} else if (test == "FULL_BY_BLOCK") {
					connectivity_cfg = new FullByBlockConnectivity(config_file, (*gp_source)->size(), (*gp_target)->size());
				} else if (test == "RANDOM_BY_BLOCK") {
					connectivity_cfg = new RandomByBlockConnectivity(config_file, (*gp_source)->size(), (*gp_target)->size());
				} else if (test == "RANDOM") {
					connectivity_cfg = new RandomConnectivity(config_file);
				} else if (test == "LIST") {
					connectivity_cfg = new ListConnectivity(config_file, (*gp_source)->size(), (*gp_target)->size());
				} else if (test == "LOOPS") {
					if (gp_source != gp_target) throw ConfigError("Network: LOOP pattern connectivity only with the same group as source and target");
					connectivity_cfg = new LoopConnectivity(config_file, (*gp_target)->size());
				} else throw ConfigError("Network: unknown type of connectivity" + error_tag + ", got '" + test + "'");
								
				// connect the groups
				(**gp_source).connect_to(**gp_target, weight_distrib_cfg, delay_distrib_cfg, syn_mech_cfg, plast_mech_cfg, connectivity_cfg, cfg_list_, nb_con);

				// cleaning
				delete weight_distrib_cfg;
				delete delay_distrib_cfg;
				delete connectivity_cfg;

				// check for end marker END_CONNECT
				if (config_file.eof()) 
					throw ConfigError("Network: wrong end of script file" + error_tag);
				config_file >> test;
				if (test != "END_CONNECT") throw ConfigError("Network: 'END_CONNECT' missing" + error_tag + ", got '" + test + "'");

				// output on screen: write out the size of the constructed group
				if (! no_output)
				{
					std::cout << nb_con << " connections from group #" << n_source << " to group #" << n_target << std::endl;
				}
			}
			else throw ConfigError("Network: uncorrect tag in script file (only 'CREATE_GROUP' and 'CONNECT' acceptable so far)");
		}
	}
	// close the config file
	config_file.close();
//	logoutfile.close();

}

void ParNetwork::config_from_file(std::string configfilename, std::string logfilename, bool no_output, Size & ncells, Size &ngroups)
{
//	std::ofstream logoutfile;
//	logoutfile.open(logfilename.c_str());
	std::ifstream config_file;
	config_file.open(configfilename.c_str());

	if ((! config_file.is_open())) // || (! logoutfile.is_open()))
		throw ConfigError("Network: script file not found or cannot create log file");
	else {
		std::string test; // use to parse the config_file
		std::string error_tag; // used to build error messages

		/////////////////////////////////////////////////
		// simulation initialisation
		if (config_file.eof()) 
			throw ConfigError("Network: expected 'SIM_INIT '(to initialise the simulation environment)");
		config_file >> test;
		if (test != "SIM_INIT") throw ConfigError("Network: expected 'SIM_INIT' (to initialise the simulation environment), got '" + test + "'");
		error_tag = " (in the SIM_INIT loop)";
		// initialise the simulation environment
		SimEnv::set(config_file);
		// check for end marker END_SIM_INIT in SimEnv::set(...)

		/////////////////////////////////////////////////
		// defines the outputs
		if (config_file.eof()) 
			throw ConfigError("Network: expected 'SET_OUTPUTS'");
		config_file >> test;
		if (test != "SET_OUTPUTS") throw ConfigError("Network: expected 'SET_OUTPUTS', got '" + test + "'");
		error_tag = " (in the SET_OUTPUTS loop)";
		// initialise the simulation environment
		OutputManager::set(config_file);

		/////////////////////////////////////////////////
		// create groups of neurons and connect them
		Size create_tag_number = 0, connect_tag_number = 0;
		Size num_gp = 0, nb_con; // for console screen outputs
		while (! config_file.eof() )
		{
			test = "";
			config_file >> test;
			if (test == "")
			{} // corresponds to an empty line in the end of the file
			else if (test == "CREATE_GROUP")
			{
				
				++create_tag_number;
				error_tag = std::string(" (in the #") + StringFormatter::IntToStr(create_tag_number) + " " + test + " loop)";
				// create a group of neurons
				gp_list_.push_back(boost::shared_ptr<Group>(new Group()));
				gp_list_.back()->populate_config(config_file);
				// check for end marker END_NRN_GROUP in Group::populate(...)

				// output on screen: write out the size of the constructed group
				if (! no_output)
				{
					std::cout << "group " << num_gp << " has " << gp_list_.back()->size() << " neurons" << std::endl;
				}
				++num_gp;
				++ngroups;
				ncells+= gp_list_.back()->size() ;
			}
// COPY_GROUP ??? clonable???
			else if (test == "CONNECT")
			{
				++connect_tag_number;
				nb_con = 0;
				error_tag = std::string(" (in the #") + StringFormatter::IntToStr(create_tag_number) + " " + test + " loop)";
				ConfigBase * syn_mech_cfg, * plast_mech_cfg;
				ConnectivityManager * connectivity_cfg;
				DistributionManager * weight_distrib_cfg, * delay_distrib_cfg;
				
				// read the two groups to be connected
				Size n_source, n_target;
				READ_FROM_FILE(config_file, n_source, "n_source", "Network" + error_tag)
				READ_FROM_FILE(config_file, n_target, "n_target", "Network" + error_tag)
				ListGroupType::iterator gp_source = gp_list_.begin();
				for (Size i = 0; i < n_source; ++i)
				{
					if (gp_source == gp_list_.end())
						throw ConfigError("Network: index of source group out of bound" + error_tag);
					++gp_source;
				}
				if (gp_source == gp_list_.end())
					throw ConfigError("Network: index of source group out of bound" + error_tag);
				ListGroupType::iterator gp_target = gp_list_.begin();
				for (Size i = 0; i < n_target; ++i)
				{
					if (gp_target == gp_list_.end())
						throw ConfigError("Network: index of target group out of bound" + error_tag);
					++gp_target;
				}
				if (gp_target == gp_list_.end())
					throw ConfigError("Network: index of target group out of bound" + error_tag);
				
				// get the weight distribution
				if (config_file.eof()) 
					throw ConfigError("Network: expected the type of weight distribution" + error_tag);
				config_file >> test;
				if (test == "WEIGHT_DELTA") {
					weight_distrib_cfg = new DeltaDistribution(config_file);
				} else if (test == "WEIGHT_UNIFORM") {
					weight_distrib_cfg = new UniformDistribution(config_file);
				} else if (test == "WEIGHT_BIMODAL") {
					weight_distrib_cfg = new BimodalDistribution(config_file);
				} else if (test == "WEIGHT_GAUSSIAN") {
					weight_distrib_cfg = new GaussianDistribution(config_file);
				} else throw ConfigError("Network: unknown type of weight distribution" + error_tag + ", got '" + test + "'");
#if 0				
				if (weight_distrib_cfg) (**gp_source).weight_distrib_cfg_.push_back(boost::shared_ptr<DistributionManager>(weight_distrib_cfg));
#endif
				// get the delay distribution
				if (config_file.eof()) 
					throw ConfigError("Network: expected the type of delay distribution" + error_tag);
				config_file >> test;
				if (test == "DELAY_DELTA") {
					delay_distrib_cfg = new DeltaDistribution(config_file);
				} else if (test == "DELAY_UNIFORM") {
					delay_distrib_cfg = new UniformDistribution(config_file);
				} else if (test == "DELAY_BIMODAL") {
					delay_distrib_cfg = new BimodalDistribution(config_file);
				} else if (test == "DELAY_GAUSSIAN") {
					delay_distrib_cfg = new GaussianDistribution(config_file);
				} else throw ConfigError("Network: unknown type of delay distribution" + error_tag + ", got '" + test + "'");
#if 0
				if (delay_distrib_cfg) (**gp_source).delay_distrib_cfg_.push_back(boost::shared_ptr<DistributionManager>(delay_distrib_cfg));
#endif
				// get the synaptic activation mechanism
				if (config_file.eof()) 
					throw ConfigError("Network: expecting the type of plasticity mechanism" + error_tag);
				config_file >> test;
				if (test == "COND_MECH") {
					syn_mech_cfg = new CondSynMechConfig(config_file);
					cfg_list_.push_back(boost::shared_ptr<ConfigBase>(syn_mech_cfg));
				} else if (test == "INSTANT_MECH") {
					syn_mech_cfg = new InstantSynMechConfig(config_file);
					cfg_list_.push_back(boost::shared_ptr<ConfigBase>(syn_mech_cfg));
				} else if (test == "KERNEL_MECH") {
					syn_mech_cfg = new DoubleExpSynMechConfig(config_file);
					cfg_list_.push_back(boost::shared_ptr<ConfigBase>(syn_mech_cfg));
				} else throw ConfigError("Network: unknown type of synaptic activation mechanism" + error_tag + ", got '" + test + "'");
				
#if 0
				if (syn_mech_cfg) (**gp_source).syn_mech_cfg_.push_back(boost::shared_ptr<ConfigBase>(syn_mech_cfg));
#endif
				// get the synaptic plasticity mechanism
				if (config_file.eof()) 
					throw ConfigError("Network: expecting the type of plasticity mechanism" + error_tag);
				config_file >> test;
				if (test == "STDP") {
					plast_mech_cfg = new STDPMechConfig<AdditiveSTDPNegExpFunction, ClipToBounds>(config_file);
					cfg_list_.push_back(boost::shared_ptr<ConfigBase>(plast_mech_cfg));
				} else if (test == "NON_PLASTIC") {
					plast_mech_cfg = new NoPlastMechConfig();
				} else throw ConfigError("Network: unknown type of plasticity mechanism" + error_tag + ", got '" + test + "'");
#if 0
				if (plast_mech_cfg) (**gp_source).plast_mech_cfg_.push_back(boost::shared_ptr<ConfigBase>(plast_mech_cfg));
#endif				
				// get the connectivity type
				if (config_file.eof()) 
					throw ConfigError("Network: expecting the type of connectivity" + error_tag);
				config_file >> test;
				if (test == "FULL") {
					connectivity_cfg = new FullConnectivity(config_file);
				} else if (test == "RANDOM_FULL_PRECON") {
					connectivity_cfg = new RandomFullPreConnectivity(config_file, (*gp_target)->size());
				} else if (test == "RANDOM_FIXED_PRECON") {
					connectivity_cfg = new RandomFixedPreConnectivity(config_file, (*gp_source)->size(), (*gp_target)->size());
				} else if (test == "FULL_BY_BLOCK") {
					connectivity_cfg = new FullByBlockConnectivity(config_file, (*gp_source)->size(), (*gp_target)->size());
				} else if (test == "RANDOM_BY_BLOCK") {
					connectivity_cfg = new RandomByBlockConnectivity(config_file, (*gp_source)->size(), (*gp_target)->size());
				} else if (test == "RANDOM") {
					connectivity_cfg = new RandomConnectivity(config_file);
				} else if (test == "LIST") {
					connectivity_cfg = new ListConnectivity(config_file, (*gp_source)->size(), (*gp_target)->size());
				} else if (test == "LOOPS") {
					if (gp_source != gp_target) throw ConfigError("Network: LOOP pattern connectivity only with the same group as source and target");
					connectivity_cfg = new LoopConnectivity(config_file, (*gp_target)->size());
				} else throw ConfigError("Network: unknown type of connectivity" + error_tag + ", got '" + test + "'");
#ifdef PARALLELSIM					
				//if (connectivity_cfg) (**gp_source).connectivity_cfg_.push_back(boost::shared_ptr<ConnectivityManager>(connectivity_cfg));
				if(syn_mech_cfg && plast_mech_cfg && connectivity_cfg && weight_distrib_cfg && delay_distrib_cfg)
					conn_list_.push_back(boost::shared_ptr<Conn>(new Conn(*gp_source, *gp_target,
						boost::shared_ptr<ConfigBase>(syn_mech_cfg),
						boost::shared_ptr<ConfigBase>(plast_mech_cfg),
						boost::shared_ptr<ConnectivityManager>(connectivity_cfg),
						boost::shared_ptr<DistributionManager>(weight_distrib_cfg),
						boost::shared_ptr<DistributionManager>(delay_distrib_cfg))));
				
#endif			
				// connect the groups
				//(**gp_source).connect_to(**gp_target, weight_distrib_cfg, delay_distrib_cfg, syn_mech_cfg, plast_mech_cfg, connectivity_cfg, cfg_list_, nb_con);

				// cleaning


				// check for end marker END_CONNECT
				if (config_file.eof()) 
					throw ConfigError("Network: wrong end of script file" + error_tag);
				config_file >> test;
				if (test != "END_CONNECT") throw ConfigError("Network: 'END_CONNECT' missing" + error_tag + ", got '" + test + "'");

				// output on screen: write out the size of the constructed group
				if (! no_output)
				{
					std::cout << nb_con << " connections from group #" << n_source << " to group #" << n_target << std::endl;
				}
			}
			else throw ConfigError("Network: uncorrect tag in script file (only 'CREATE_GROUP' and 'CONNECT' acceptable so far)");
		}
	}
	// close the config file
	config_file.close();
//	logoutfile.close();

}

void ParNetwork::create()
{
	for (ListGroupType::const_iterator i = gp_list_.begin(); 
		 i != gp_list_.end(); 
		 ++i)
		(*i)->create_population(); 



		std::string test; // use to parse the config_file
		std::string error_tag; // used to build error messages




/*	for (ListConnType::const_iterator i = conn_list_.begin(); 
		 i != conn_list_.end(); 
		 ++i)
		(*i)->connect_to(); 
// connect the groups
		(**gp_source).par_connect_to(**gp_target, weight_distrib_cfg, delay_distrib_cfg, syn_mech_cfg, plast_mech_cfg, connectivity_cfg, cfg_list_, nb_con);
*/

		/////////////////////////////////////////////////
		// create groups of neurons and connect them
		Size create_tag_number = 0, connect_tag_number = 0;
		Size num_gp = 0, nb_con; // for console screen outputs


// COPY_GROUP ??? clonable???
/*			else if (test == "CONNECT")
			{
				++connect_tag_number;
				nb_con = 0;
				error_tag = std::string(" (in the #") + StringFormatter::IntToStr(create_tag_number) + " " + test + " loop)";
				ConfigBase * syn_mech_cfg, * plast_mech_cfg;
				ConnectivityManager * connectivity_cfg;
				DistributionManager * weight_distrib_cfg, * delay_distrib_cfg;
				
				// read the two groups to be connected
				Size n_source, n_target;
				READ_FROM_FILE(config_file, n_source, "n_source", "Network" + error_tag)
				READ_FROM_FILE(config_file, n_target, "n_target", "Network" + error_tag)
				ListGroupType::iterator gp_source = gp_list_.begin();
				for (Size i = 0; i < n_source; ++i)
				{
					if (gp_source == gp_list_.end())
						throw ConfigError("Network: index of source group out of bound" + error_tag);
					++gp_source;
				}
				if (gp_source == gp_list_.end())
					throw ConfigError("Network: index of source group out of bound" + error_tag);
				ListGroupType::iterator gp_target = gp_list_.begin();
				for (Size i = 0; i < n_target; ++i)
				{
					if (gp_target == gp_list_.end())
						throw ConfigError("Network: index of target group out of bound" + error_tag);
					++gp_target;
				}
				if (gp_target == gp_list_.end())
					throw ConfigError("Network: index of target group out of bound" + error_tag);
				
				// get the weight distribution
				if (config_file.eof()) 
					throw ConfigError("Network: expected the type of weight distribution" + error_tag);
				config_file >> test;
				if (test == "WEIGHT_DELTA") {
					weight_distrib_cfg = new DeltaDistribution(config_file);
				} else if (test == "WEIGHT_UNIFORM") {
					weight_distrib_cfg = new UniformDistribution(config_file);
				} else if (test == "WEIGHT_BIMODAL") {
					weight_distrib_cfg = new BimodalDistribution(config_file);
				} else if (test == "WEIGHT_GAUSSIAN") {
					weight_distrib_cfg = new GaussianDistribution(config_file);
				} else throw ConfigError("Network: unknown type of weight distribution" + error_tag + ", got '" + test + "'");
				
				// get the delay distribution
				if (config_file.eof()) 
					throw ConfigError("Network: expected the type of delay distribution" + error_tag);
				config_file >> test;
				if (test == "DELAY_DELTA") {
					delay_distrib_cfg = new DeltaDistribution(config_file);
				} else if (test == "DELAY_UNIFORM") {
					delay_distrib_cfg = new UniformDistribution(config_file);
				} else if (test == "DELAY_BIMODAL") {
					delay_distrib_cfg = new BimodalDistribution(config_file);
				} else if (test == "DELAY_GAUSSIAN") {
					delay_distrib_cfg = new GaussianDistribution(config_file);
				} else throw ConfigError("Network: unknown type of delay distribution" + error_tag + ", got '" + test + "'");

				// get the synaptic activation mechanism
				if (config_file.eof()) 
					throw ConfigError("Network: expecting the type of plasticity mechanism" + error_tag);
				config_file >> test;
				if (test == "COND_MECH") {
					syn_mech_cfg = new CondSynMechConfig(config_file);
					cfg_list_.push_back(boost::shared_ptr<ConfigBase>(syn_mech_cfg));
				} else if (test == "INSTANT_MECH") {
					syn_mech_cfg = new InstantSynMechConfig(config_file);
					cfg_list_.push_back(boost::shared_ptr<ConfigBase>(syn_mech_cfg));
				} else if (test == "KERNEL_MECH") {
					syn_mech_cfg = new DoubleExpSynMechConfig(config_file);
					cfg_list_.push_back(boost::shared_ptr<ConfigBase>(syn_mech_cfg));
				} else throw ConfigError("Network: unknown type of synaptic activation mechanism" + error_tag + ", got '" + test + "'");
				
				// get the synaptic plasticity mechanism
				if (config_file.eof()) 
					throw ConfigError("Network: expecting the type of plasticity mechanism" + error_tag);
				config_file >> test;
				if (test == "STDP") {
					plast_mech_cfg = new STDPMechConfig<AdditiveSTDPNegExpFunction, ClipToBounds>(config_file);
					cfg_list_.push_back(boost::shared_ptr<ConfigBase>(plast_mech_cfg));
				} else if (test == "NON_PLASTIC") {
					plast_mech_cfg = new NoPlastMechConfig();
				} else throw ConfigError("Network: unknown type of plasticity mechanism" + error_tag + ", got '" + test + "'");

				// get the connectivity type
				if (config_file.eof()) 
					throw ConfigError("Network: expecting the type of connectivity" + error_tag);
				config_file >> test;
				if (test == "FULL") {
					connectivity_cfg = new FullConnectivity(config_file);
				} else if (test == "RANDOM_FULL_PRECON") {
					connectivity_cfg = new RandomFullPreConnectivity(config_file, (*gp_target)->size());
				} else if (test == "RANDOM_FIXED_PRECON") {
					connectivity_cfg = new RandomFixedPreConnectivity(config_file, (*gp_source)->size(), (*gp_target)->size());
				} else if (test == "FULL_BY_BLOCK") {
					connectivity_cfg = new FullByBlockConnectivity(config_file, (*gp_source)->size(), (*gp_target)->size());
				} else if (test == "RANDOM_BY_BLOCK") {
					connectivity_cfg = new RandomByBlockConnectivity(config_file, (*gp_source)->size(), (*gp_target)->size());
				} else if (test == "RANDOM") {
					connectivity_cfg = new RandomConnectivity(config_file);
				} else if (test == "LIST") {
					connectivity_cfg = new ListConnectivity(config_file, (*gp_source)->size(), (*gp_target)->size());
				} else if (test == "LOOPS") {
					if (gp_source != gp_target) throw ConfigError("Network: LOOP pattern connectivity only with the same group as source and target");
					connectivity_cfg = new LoopConnectivity(config_file, (*gp_target)->size());
				} else throw ConfigError("Network: unknown type of connectivity" + error_tag + ", got '" + test + "'");
								
				// connect the groups
				(**gp_source).connect_to(**gp_target, weight_distrib_cfg, delay_distrib_cfg, syn_mech_cfg, plast_mech_cfg, connectivity_cfg, cfg_list_, nb_con);

				// cleaning
				delete weight_distrib_cfg;
				delete delay_distrib_cfg;
				delete connectivity_cfg;

				// check for end marker END_CONNECT
				if (config_file.eof()) 
					throw ConfigError("Network: wrong end of script file" + error_tag);
				config_file >> test;
				if (test != "END_CONNECT") throw ConfigError("Network: 'END_CONNECT' missing" + error_tag + ", got '" + test + "'");

				// output on screen: write out the size of the constructed group
				if (! no_output)
				{
					std::cout << nb_con << " connections from group #" << n_source << " to group #" << n_target << std::endl;
				}
			}
			else throw ConfigError("Network: uncorrect tag in script file (only 'CREATE_GROUP' and 'CONNECT' acceptable so far)");
		
*/

}


