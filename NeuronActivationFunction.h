// NeuronActivationFunction.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NEURONACTIVATIONFUNCTION_H
#define NEURONACTIVATIONFUNCTION_H

#include <math.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// FuncIdentity struct definition
// x -> x
///////////////////////////////////////////////////////////////////////////////////////////////////

struct FuncIdentity {
    inline static const double & func(const double & x) {
        return x;
    }
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// FuncLinearThreshold struct definition
// x -> max(THRESHOLD_MIN, min(x, THRESHOLD_MAX))
///////////////////////////////////////////////////////////////////////////////////////////////////

struct FuncLinearThreshold {
    inline static const double & func(const double & x) {
        return (x < THRESHOLD_MIN) ? THRESHOLD_MIN : ((x > THRESHOLD_MAX) ? THRESHOLD_MAX : x);
    }
    static const double THRESHOLD_MIN;
    static const double THRESHOLD_MAX;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// FuncSigmoid struct definition
// x -> 1 - exp(-x)
// SATURATED_RATE is the max rate
///////////////////////////////////////////////////////////////////////////////////////////////////

#define SATURATED_RATE 30

struct FuncSigmoid {
    inline static double func(const double & x) {
        return (x < 0) ? 0 : SATURATED_RATE - SATURATED_RATE * exp(-x / SATURATED_RATE);
    }
};

#undef SATURATED_RATE



///////////////////////////////////////////////////////////////////////////////////////////////////
// FuncSigmoidSquare struct definition
// x -> 1 - exp(-x^2)
// SATURATED_RATE is the max rate, SATURATED_RATE_SQUARE_DOUBLE must be the double of the square of SATURATED_RATE
///////////////////////////////////////////////////////////////////////////////////////////////////

#define SATURATED_RATE 30
#define SATURATED_RATE_SQUARE_DOUBLE 1800

struct FuncSigmoidSquare {
    inline static double func(const double & x) {
        return (x < 0) ? 0 : SATURATED_RATE - SATURATED_RATE * exp(-x * x / SATURATED_RATE_SQUARE_DOUBLE);
    }
};

#undef SATURATED_RATE
#undef SATURATED_RATE_SQUARE_DOUBLE



#endif // !defined(NEURONACTIVATIONFUNCTION_H)
