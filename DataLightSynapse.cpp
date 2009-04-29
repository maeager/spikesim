// DataLightSynapse.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "DataLightSynapse.h"
#include "SimulationEnvironment.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// DataIndivSynapseConfig function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// constructor
DataLightSynapseConfig::DataLightSynapseConfig(DistributionManager * weightdistrib, DistributionManager * delaydistrib)
	: weight_(weightdistrib->generate_value())
	, delay_(delaydistrib->generate_value())
{
	if ((delay_ < SimEnv::timestep()) || (delay_ > SimEnv::i_max_delay() * SimEnv::timestep()))
		throw ConfigError("DataIndivSynapse: delay smaller than the time step or greater than the maximal value (cf. simulation environment SimEnv)");
}
