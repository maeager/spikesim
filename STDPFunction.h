// STDPFunction.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef STDPFUNCTION_H
#define STDPFUNCTION_H


#include <math.h>

#include "GlobalDefs.h"


//!	STDPMechConfig: mechanism configurator for STDPMech.
/*!	AdditiveSTDPNegExpFunction class definition
    One negative exponential for each branch (depression and potentiation).
    Weight change for potentiation (dt > 0): +cP_ * exp(- dt * inv_tauP_) 
    Weight change for depression (dt < 0): +cD_ * exp(dt * inv_tauD_)
    No weight dependence.
 */
class AdditiveSTDPNegExpFunction
{
protected:
    //! Sets the parmeters of the function.
	/*!	Called by the constructor of STDPMech.
	 */
    void set_params(double tauP, double tauD, double cP, double cD)
    {
        inv_tauP_ = 1 / tauP;
    	inv_tauD_ = 1 / tauD;
    	cP_ = cP;
    	cD_ = cD;
    	cutoffP_ = 5 * tauP;
    	cutoffD_ = 5 * tauD;
        mass_ = tauP * cP + tauD * cD;
    }

    // a virer
	double operator() (const Time & t1, const Time & t2)
	{
		return  (t1<t2)?
			cD_ * exp((t1-t2)* inv_tauD_) :
				(t1>t2)?
			cP_ * exp((t2-t1) * inv_tauP_) :
			0.;
	}
	
	inline double weight_change_impl(double & weight, const double & dt) const
    {
        return (dt == 0)? 0. : (dt > 0)? cP_ * exp(- dt * inv_tauP_) : cD_ * exp(dt * inv_tauD_);
    }

	double inv_tauP_	/*!< Potentiation time constant */
         , inv_tauD_    /*!< Depression time constant */
		 , cP_          /*!< Potentiation scaling coefficient */
		 , cD_          /*!< Depression scaling coefficient */
		 , cutoffP_     /*!< Potentiation cutoff */
		 , cutoffD_     /*!< Depression cutoff */
    	 , mass_;       /*!< integral value of the function */
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// ClipToBounds class definition
///////////////////////////////////////////////////////////////////////////////////////////////////

class ClipToBounds
{
protected:
    //! Sets the parmeters of the function.
	/*!	Called by the constructor of STDPMech.
	 */
    void set_params(double gmin, double gmax)
    {
        min_ = gmin;
        max_ = gmax;
    }
    //! Sets the parmeters of the function.
	/*!	Called by the constructor of STDPMech.
	 */
    void check_bounds_impl(double & weight) const
    {
    	if (weight > max_)
           weight = max_;
    	else if (weight < min_)
           weight = min_;
    }

	double min_
		 , max_;     
};



#endif // !defined(OUTPUTMANAGER_H

