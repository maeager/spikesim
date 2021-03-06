// DistributionManager.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Macros.h"
#include "DistributionManager.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// DeltaDistribution function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// DeltaDistribution constructor with script file
DeltaDistribution::DeltaDistribution(std::ifstream & is)
{
    std::string test;
    READ_FROM_FILE(is, value_, "value", "DeltaDistribution")
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// UniformDistribution function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// UniformDistribution constructor with script file
UniformDistribution::UniformDistribution(std::ifstream & is)
{
    std::string test;
    READ_FROM_FILE(is, mean_, "mean", "UniformDistribution")
    READ_FROM_FILE(is, spread_, "spread", "UniformDistribution")
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// BimodalDistribution function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// BimodalDistribution constructor with script file
BimodalDistribution::BimodalDistribution(std::ifstream & is)
{
    std::string test;
    READ_FROM_FILE(is, a_, "a", "BimodalDistribution")
    READ_FROM_FILE(is, b_, "b", "BimodalDistribution")
    READ_FROM_FILE(is, proba_, "proba", "BimodalDistribution")
    if (proba_ < 0 || proba_ > 1) throw ConfigError("BimodalDistribution: probability not in [0,1]");
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// GaussianDistribution function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// GaussianDistribution constructor with script file
GaussianDistribution::GaussianDistribution(std::ifstream & is)
{
    std::string test;
    READ_FROM_FILE(is, mean_, "mean", "GaussianDistribution")
    READ_FROM_FILE(is, variance_, "variance", "GaussianDistribution")
}


