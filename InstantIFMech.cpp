// InstantIFMech.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <fstream>

#include "Macros.h"
#include "InstantIFMech.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// InstantIFMechConfig function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
/////////////////////////////////////////////////
// InstantIFMechConfig constructor
InstantIFMechConfig::InstantIFMechConfig(double tauP, double Vth, double Vr, double Vp)
	: tauP_(tauP)
	, Vth_(Vth)
	, Vr_(Vr)
	, Vp_(Vp)
	, Vfactor_(1.0 - SimEnv::timestep()/tauP)
{
}
*/
/////////////////////////////////////////////////
// InstantIFMechConfig constructor with script file
InstantIFMechConfig::InstantIFMechConfig(std::ifstream & is)
{
	std::string test;
	READ_FROM_FILE(is, tauP_, "tau", "InstantIFMechConfig")
	READ_FROM_FILE(is, Vth_, "Vth", "InstantIFMechConfig")
	READ_FROM_FILE(is, Vr_, "Vr", "InstantIFMechConfig")
	READ_FROM_FILE(is, Vp_, "Vp", "InstantIFMechConfig")
	Vfactor_ = 1.0 - SimEnv::timestep()/tauP_;

	// under development:
	// get the V_init distribution
	std::string error_tag; // used to build error messages
	if (is.eof()) 
		throw ConfigError("InstantIFMechConfig: expected the type of V_init distribution" + error_tag);
	is >> test;
	if (test == "V_INIT_DELTA") {
		V_init_distrib_ = new DeltaDistribution(is);
	} else if (test == "V_INIT_UNIFORM") {
		V_init_distrib_ = new UniformDistribution(is);
	} else if (test == "V_INIT_BIMODAL") {
		V_init_distrib_ = new BimodalDistribution(is);
	} else if (test == "V_INIT_GAUSSIAN") {
		V_init_distrib_ = new GaussianDistribution(is);
	} else throw ConfigError("InstantIFMechConfig: unknown type of V_init distribution" + error_tag);
}

/////////////////////////////////////////////////
// stream operator overloading
std::ofstream & operator<<(std::ofstream & os, const InstantIFMechConfig & ifgc)
{
	os << "tauP " << ifgc.tauP()
		<< "; Vth " << ifgc.Vth() 
		<< "; Vr " << ifgc.Vr() 
		<< "; Vp " << ifgc.Vp() 
		<< ";";
	return os;
}
