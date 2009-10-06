// CondSynMech.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <fstream>

#include "Macros.h"
#include "CondSynMech.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// CondSynMechConfig function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
/////////////////////////////////////////////////
// CondSynMechConfig constructor
CondSynMechConfig::CondSynMechConfig(double tauA, double tauB, double Vreversal)
    : factorA_(exp( -SimEnv::timestep() / tauA ))
    , factorB_(exp( -SimEnv::timestep() / tauB ))
    , Vreversal_(Vreversal)
#ifdef _DEBUG
    , tauA_(tauA)
    , tauB_(tauB)
#endif
{
}
*/
/////////////////////////////////////////////////
// CondSynMechConfig constructor with script file
CondSynMechConfig::CondSynMechConfig(std::ifstream & is)
{
    double tauA, tauB;
    std::string test;
    READ_FROM_FILE(is, tauA, "tauA", "CondSynMechConfig")
    READ_FROM_FILE(is, tauB, "tauB", "CondSynMechConfig")
    READ_FROM_FILE(is, Vreversal_, "Vrev", "CondSynMechConfig")
#ifdef _DEBUG
    tauA_ = tauA;
    tauB_ = tauB;
#endif
    factorA_ = exp(-SimEnv::timestep() / tauA);
    factorB_ = exp(-SimEnv::timestep() / tauB);
}

/////////////////////////////////////////////////
// stream operator overloading
std::ofstream & operator<<(std::ofstream & os, const CondSynMechConfig & csmcfg)
{
    os << "tauA " << -SimEnv::timestep() / log(csmcfg.factorA())
    << "tauB " << -SimEnv::timestep() / log(csmcfg.factorB())
    << "; Vreversal " << csmcfg.Vreversal()
    << ";";
    return os;
}
