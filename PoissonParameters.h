// PoissonParameter.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef POISSONPARAMETERS_H
#define POISSONPARAMETERS_H

//#include <boost/numeric/interval/constants.hpp>
#include "SimulationEnvironment.h"
#include "ManageableInput.h"


//! Encapsulates a constant parameter.
/*! Used by PoissonMechConfig.
 */
class ConstantPoissonParameter
{
protected:
    //! Constructor.
    /*! \param is stream of data (STL ifstream) to initialise the configurator.
     */
    ConstantPoissonParameter(std::ifstream & is) {
        std::string test;
        READ_FROM_FILE(is, spontaneous_rate_, "spontaneous_rate", "ConstantPoissonParameter")
    }

    double spontaneous_rate_; /*!< Spontaneous rate. */
};


//! Encapsulates a constant parameter.
/*! Used by PoissonMechConfig.
 */
class OscillatoryPoissonParameter
        : public ManageableInput
{
protected:
    OscillatoryPoissonParameter(std::ifstream & is) {
        std::string test;
        READ_FROM_FILE(is, constant_, "constant", "OscillatoryPoissonParameter")
        READ_FROM_FILE(is, amplitude_, "amplitude", "OscillatoryPoissonParameter")
        double freq;
        READ_FROM_FILE(is, freq, "freq", "OscillatoryPoissonParameter")
        omega_ = 3.14159265358979 * 2 * freq;
        READ_FROM_FILE(is, phase_, "phase", "OscillatoryPoissonParameter")
    }
    double spontaneous_rate_;
private:
    void input_update() {
        spontaneous_rate_ = constant_ + amplitude_ * sin(omega_ * SimEnv::sim_time() + phase_);
    }
    double constant_;
    double amplitude_;
    double omega_;
    double phase_;
};





#endif // !defined(POISSONPARAMETERS_H)
