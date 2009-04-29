// DistributionManager.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DISTRIBUTIONMANAGER_H
#define DISTRIBUTIONMANAGER_H

#include <fstream>

#include "Error.h"
#include "RandomGenerator.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// DistributionManager mother class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

struct DistributionManager
{
	virtual double generate_value() = 0; 
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// DeltaDistribution class definition
// generates values equal to the value
///////////////////////////////////////////////////////////////////////////////////////////////////

class DeltaDistribution
	: public DistributionManager
{
public:
	DeltaDistribution(std::ifstream & is);
	double generate_value() {return value_;}
protected:
	double value_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// UniformDistribution class definition
// generates values uniformly distributed in [mean - spread, mean + spread]
///////////////////////////////////////////////////////////////////////////////////////////////////

class UniformDistribution
	: public DistributionManager
{
public:
	UniformDistribution(double mean, double spread) : mean_(mean), spread_(spread) {}
	UniformDistribution(std::ifstream & is);
	double generate_value() {return mean_ - spread_ + 2 * spread_ * RandomGenerator::dran(1.);}
protected:
	double mean_, spread_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// BimodalDistribution class definition
// return a_ with probability proba_ and b_ with (1-proba_)
///////////////////////////////////////////////////////////////////////////////////////////////////

class BimodalDistribution
	: public DistributionManager
{
public:
	BimodalDistribution(std::ifstream & is);
	double generate_value() {return (RandomGenerator::dran(1.) < proba_)? a_ : b_;}
protected:
	double a_, b_, proba_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// GaussianDistribution class definition
// generates values gaussianly distributed around the mean with the given variance
///////////////////////////////////////////////////////////////////////////////////////////////////

class GaussianDistribution
	: public DistributionManager
{
public:
	GaussianDistribution(std::ifstream & is);
	double generate_value() {return mean_ + RandomGenerator::gran(variance_);}
protected:
	double mean_, variance_;
};




#endif // !defined(DISTRIBUTIONMANAGER_H)
