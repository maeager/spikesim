// DataPlastNeuron.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "DataPlastNeuron.h"


PlasticityManager::ListPlastNrnType PlasticityManager::list_;



void PlasticityManager::append(DataPlastNeuron * const pt_nrn)
{
	list_.push_back(pt_nrn);
}
