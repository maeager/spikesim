// InstantSynMech.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <fstream>

#include "Macros.h"
#include "InstantSynMech.h"

#include <math.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// InstantSynMechConfig function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
/////////////////////////////////////////////////
// InstantSynMechConfig constructor
InstantSynMechConfig::InstantSynMechConfig(double Vreversal, double BkgInpRate, double BkgInpWeight)
    : Vreversal_(Vreversal), BkgInpRate_(BkgInpRate), BkgInpWeight_(BkgInpWeight)
{
    MakeCondProbTable();
}
*/
/////////////////////////////////////////////////
// InstantSynMechConfig constructor with script file
InstantSynMechConfig::InstantSynMechConfig(std::ifstream & is)
{
    std::string test;
    READ_FROM_FILE(is, Vreversal_, "Vrev", "InstantSynMechConfig")
    READ_FROM_FILE(is, BkgInpRate_, "BkgInpRate", "InstantSynMechConfig")
    READ_FROM_FILE(is, BkgInpWeight_, "BkgInpWeight", "InstantSynMechConfig")
    MakeCondProbTable();
}

/////////////////////////////////////////////////
// Poisson distributed background input
void InstantSynMechConfig::MakeCondProbTable()
{
    double lambda = BkgInpRate_ * SimEnv::timestep();
    double epsilon = 1.0e-14;

    int n = 0;
    double P_ge_n = 1.0;  // Pr(k>=0)
    double PoissonProb_n = exp(-lambda); // Pr(k==0);

    while ((n < CP_size - 1) && (P_ge_n > epsilon)) {
        // Pr(k==n|k>=n) = Pr(k==n) / Pr(k>=n):
        CondProb[n] = PoissonProb_n / P_ge_n;
        if (CondProb[n] > 1)
            break;
        ++n;
        P_ge_n -= PoissonProb_n;
        PoissonProb_n *= lambda / n;
    }
    CondProb[n] = 1;
    if (n == CP_size - 1) {
        std::cout << "Warning: Poisson Table may be inaccurate; neglected distribution = " << P_ge_n;
        std::cout << std::endl;
    }
}

double InstantSynMechConfig::poisson_increment() const
{
    int n = 0;
    // This loop always terminates because there is a largest possible n with CondProb[n]=1
    while (1) {
        if (RandomGenerator::dran(1.0) <= CondProb[n])
            break;
        n++;
//      if (n==CP_size)
//          std::cout << "Error: CondProb[" << n-1 << "]=" << CondProb[n-1];
    }

    return n * BkgInpWeight_;
}

/////////////////////////////////////////////////
// stream operator overloading
std::ofstream & operator<<(std::ofstream & os, const InstantSynMechConfig & csmcfg)
{
    os << "Vreversal " << csmcfg.Vreversal() << ";";
    return os;
}
