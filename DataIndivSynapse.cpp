// DataIndivSynapse.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "DataIndivSynapse.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// DataIndivSynapseConfig function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// constructor
DataIndivSynapseConfig::DataIndivSynapseConfig(DistributionManager * weightdistrib
        , DistributionManager * delaydistrib)
        : weightdistrib_(weightdistrib)
        , delaydistrib_(delaydistrib)
{
}
