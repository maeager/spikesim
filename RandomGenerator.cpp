// RandomGenerator.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "RandomGenerator.h"



bool RandomGenerator::initialised_ = false;

/*
bool RandomGenerator::probability_trial_for_time_step(volatile double & rate)
{
	if ( ran() < rate*SimEnv::timestep() )
		return true;
	else 
		return false;
}
*/

bool RandomGenerator::
+(const double & rate)
{
	if ( ran() < rate*SimEnv::timestep() )
		return true;
	else 
		return false;
}

int RandomGenerator::iran(const int & n)
{
	// return a random number between 0 and n-1
	return (int) floor(ran()*n);
}

double RandomGenerator::dran(const double & max)
{
	// return a random number between 0 and max
	return ran() * max;
}

double RandomGenerator::gran(const double & sigma)
{
	// return a random number with a gaussian distribution around 0 and of variance sigma
	double x, y, r2;
	do
	{
		/* choose x,y in uniform square (-1,-1) to (+1,+1) */

		x = -1 + 2 * ran();
		y = -1 + 2 * ran();

		/* see if it is in the unit circle */
		r2 = x * x + y * y;
	}
	while (r2 > 1.0 || r2 == 0);

	/* Box-Muller transform */
	return sigma * y * sqrt(-2.0 * log (r2) / r2);
}
