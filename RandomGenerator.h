// RandomGenerator.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include <math.h>

#include "SimulationEnvironment.h"


#define MBIG 1000000000
#define MSEED 161803398
#define MZ 0
#define FAC (1.0/MBIG)


///////////////////////////////////////////////////////////////////////////////////////////////////
//! RandomGenerator class definition
// to encapsulate all the random number generators
///////////////////////////////////////////////////////////////////////////////////////////////////
class RandomGenerator
{
public:
    static void reinit() {
        initialised_ = false;
    }
//  static bool probability_trial_for_time_step(volatile double &);
    static bool probability_trial_for_time_step(const double &);
    static int iran(const int &);
    static double dran(const double &);
    static double gran(const double &);
private:
    static double ran();
    static bool initialised_;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// RandomGenerator inline definition
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// generator
// according to Knuth, any large MBIG, and any smaller (but still large) MSEED can be substituted for the above values
// returns a uniform pseudo-random variate between 0 and 1.
// set 'initialised' to false to reinitialize or reinitialize the sequence
inline double RandomGenerator::ran()
{
    // the value 56 (range ma_[1..55]) is special and should not be modified; see Knuth
    static int inext_, inextp_;
    static long ma_[56];
    long mj, mk;
    int i, ii, k;
    if (! initialised_) {
        // Initialization
        mj = fabs(MSEED - fabs(SimEnv::random_init_seed()));
        // Initialize ma_[55] using the seed idum and the large number MSEED
        mj %= MBIG;
        ma_[55] = mj;
        mk = 1;
        for (i = 1; i <= 54; ++i) {
            ii = (21 * i) % 55;
            ma_[ii] = mk;
            mk = mj - mk;
            if (mk < MZ) mk += MBIG;
            mj = ma_[ii];
        }
        for (k = 1; k <= 4; ++k)
            for (i = 1; i <= 55; ++i) {
                ma_[i] -= ma_[1+(i+30) % 55];
                if (ma_[i] < MZ) ma_[i] += MBIG;
            }
        inext_ = 0;
        inextp_ = 31;
        initialised_ = true;
    }

    if (++inext_ == 56) inext_ = 1;
    if (++inextp_ == 56) inextp_ = 1;
    mj = ma_[inext_] - ma_[inextp_];
    if (mj < MZ) mj += MBIG;
    ma_[inext_] = mj;
    return mj*FAC;
}



#undef MBIG
#undef MSEED
#undef MZ
#undef FAC

#endif // !defined(RANDOMGENERATOR_H)
