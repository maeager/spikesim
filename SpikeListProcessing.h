// SpikeListProcessing.h
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef SPIKELISTPROCESSING_H
#define SPIKELISTPROCESSING_H


#include "OutputManager.h"

/*

///////////////////////////////////////////////////////////////////////////////////////////////////
// definition of SpikeListMean class
// computes the average (over time) firing rate of a spike train between t_start and t_stop
// the spike trains are of type std::deque<Time>
///////////////////////////////////////////////////////////////////////////////////////////////////

class SpikeListMean
{
public:
	double operator() (const std::deque<Time> & spike_train, const Time & t_start, const Time & t_stop)
	{
		std::deque<Time>::const_iterator i = spike_train.begin();
		for ( ; i != spike_train.end();
			 ++i) 
			if ((*i) >= t_start) break;
		std::deque<Time>::const_iterator j = i;
		for ( ; j != spike_train.end();
			 ++j)
			if ((*j) > t_stop) break;
		return (((double)(j-i)) / (t_stop - t_start));
	}
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// definition of SpikeListCorrel class
// computes the correlation (over time) between 2 spike trains for all puls pairs between t_start and t_stop
// the spike trains are of type std::deque<Time>
// KernelFunction is a function with (const Time &, const Time &) in argument that returns a double
///////////////////////////////////////////////////////////////////////////////////////////////////

template <class KernelFunction>
class SpikeListCorrel
{
public:
	double operator() (const std::deque<Time> & spike_train1, const std::deque<Time> & spike_train2, const Time & t_start, const Time & t_stop)
	{
		double res = 0.;

		std::deque<Time>::const_iterator i1 = spike_train1.begin();
		for ( ; i1 != spike_train1.end(); ++i1) 
			if ((*i1) >= t_start) break;
		std::deque<Time>::const_iterator i2 = spike_train2.begin();
		for ( ; i2 != spike_train2.end(); ++i2) 
			if ((*i2) >= t_start) break;
		if (i2 == spike_train2.end()) return res; // no significant spike in spike_train2

		for ( ; i1 != spike_train1.end(); ++i1)
		{
			if ((*i1) >= t_stop) break;
			for ( ; i2 != spike_train2.begin(); --i2)
			{
				if (i2 != spike_train2.end()) 
					if (((*i2) < (*i1) - KernelFunction::cutoff_left) || ((*i2) < t_start))
					{
						++i2;
						break;
					}
			}
			for ( ; i2 != spike_train2.end(); ++i2)
			{
				if (((*i2) < (*i1) + KernelFunction::cutoff_right) && ((*i2) < t_stop))
					res += kf_(*i1, *i2);
				else
					break;
			}
		}
		return res / (t_stop - t_start) 
			- KernelFunction::mass * slm_(spike_train1, t_start, t_stop) * slm_(spike_train2, t_start, t_stop);
	}
protected:
	KernelFunction kf_;
	SpikeListMean slm_;
};


*/

#endif  //SPIKELISTPROCESSING_H
