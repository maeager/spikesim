// DoubleExpSynMech.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <fstream>

#include "Macros.h"
#include "DoubleExpSynMech.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// DoubleExpSynMechConfig function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
/////////////////////////////////////////////////
// DoubleExpSynMechConfig constructor
DoubleExpSynMechConfig::DoubleExpSynMechConfig(double tauA, double tauB)
	: factorA_(exp( - SimEnv::timestep() / tauA ))
	, factorB_(exp( - SimEnv::timestep() / tauB ))
	, renorm_(1/(tauB - tauA))
#ifdef _DEBUG
	, tauA_(tauA)
	, tauB_(tauB)
#endif
{
	if (renorm_<0) throw ConfigError("DoubleExpSynMechConfig: invalid parameters, tauB < tauA or negative");;
}
*/
/////////////////////////////////////////////////
// DoubleExpSynMechConfig constructor with script file
DoubleExpSynMechConfig::DoubleExpSynMechConfig(std::ifstream & is)
{
	double tauA, tauB;
	std::string test;
	READ_FROM_FILE(is, tauA, "tauA", "DoubleExpSynMechConfig")
	READ_FROM_FILE(is, tauB, "tauB", "DoubleExpSynMechConfig")
#ifdef _DEBUG
	tauA_ = tauA;
	tauB_ = tauB;
#endif
	factorA_ = exp( -SimEnv::timestep() / tauA );
	factorB_ = exp( -SimEnv::timestep() / tauB );
	renorm_ = 1/(tauB - tauA);
	if (renorm_<0) throw ConfigError("DoubleExpSynMechConfig: invalid parameters, tauB < tauA or negative");;
}

/////////////////////////////////////////////////
// stream operator overloading
std::ofstream & operator<<(std::ofstream & os, const DoubleExpSynMechConfig & asmcfg)
{
	os << "tauA " << -SimEnv::timestep() / log(asmcfg.factorA()) 
		<< "; tauB " << -SimEnv::timestep() / log(asmcfg.factorB())
		<< ";";
	return os;
}
