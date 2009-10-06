// IFMech.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <fstream>
#include <math.h>

#include "Macros.h"
#include "IFMech.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// IFMechConfig function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
/////////////////////////////////////////////////
// IFMechConfig constructor
IFMechConfig::IFMechConfig(double tauP, double Vth, double Vr, double Vp, double refrac_per_in_seconds)
    : tauP_(tauP)
    , Vth_(Vth)
    , Vr_(Vr)
    , Vp_(Vp)
    , refrac_per_((Size) floor(refrac_per_in_seconds / SimEnv::timestep()))
{
}
*/
/////////////////////////////////////////////////
// IFMechConfig constructor with script file
IFMechConfig::IFMechConfig(std::ifstream & is)
{
    std::string test;
    READ_FROM_FILE(is, tauP_, "tau", "IFMechConfig")
    READ_FROM_FILE(is, Vth_, "Vth", "IFMechConfig")
    READ_FROM_FILE(is, Vr_, "Vr", "IFMechConfig")
    READ_FROM_FILE(is, Vp_, "Vp", "IFMechConfig")
    double refrac_per_in_seconds;
    READ_FROM_FILE(is, refrac_per_in_seconds, "refrac_per", "IFMechConfig")
    refrac_per_ = (Size) floor(refrac_per_in_seconds / SimEnv::timestep());
}

/////////////////////////////////////////////////
// stream operator overloading
std::ofstream & operator<<(std::ofstream & os, const IFMechConfig & ifgc)
{
    os << "tauP " << ifgc.tauP()
    << "; Vth " << ifgc.Vth()
    << "; Vr " << ifgc.Vr()
    << "; Vp " << ifgc.Vp()
    << "; refrac_per " << ifgc.refrac_per()
    << ";";
    return os;
}
