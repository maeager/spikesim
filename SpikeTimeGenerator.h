// SpikeTimeGenerator.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SPIKETIMEGENERATOR_H
#define SPIKETIMEGENERATOR_H


#include "DistributionManager.h"

// generator of spike times within [0,delta T[
struct SpikeTimeGenerator {
    static UniformDistribution gen_;
};



#endif // !defined(SPIKETIMEGENERATOR_H)
