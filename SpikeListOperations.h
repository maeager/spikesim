// SpikeListOperations.h
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef SPIKELISTOPERATIONS_H
#define SPIKELISTOPERATIONS_H


#include <deque>

#include "GlobalDefs.h"



//! Outputs the spike times individually.
/*!	This class has a protected constructor, and shall only be called by OutputterImpl.
 */
class IndivSpikeOutputter
	: public DataRecordNeuronVisitor
{
protected:
	//!	Type for initialisation value.
	typedef void * InitValue;
	//! Constructor.
	IndivSpikeOutputter(InitValue) {}
public:
	//!	Returns 1, not used here.
	/*!	Used by a class \b NeuronListHandler (see OutputterImpl) to determine the number of outputs to pass to a \b FileWrapper.
	 */
	unsigned nb_outputs_for_neuron(NeuronInterface * const)
	{
		return 1;
	}

	//!	Performs the outputting of the individual spikes of a neuron.
	/*! Method template called according to the real type of FileWrapper.
		\param t_start Start time. Anterior spike times are ignored.
		\param t_stop Stop time. Posterior spike times are ignored.
		\param nrn Neuron to treat.
		\param file_wrapper (of type \b FileWrapper).
	 */
	template <class FileWrapper>
	inline void operation(const Time & t_start, const Time & t_stop
						, NeuronInterface * const nrn
						, FileWrapper * const file_wrapper)
	{
		// to retrieve the list of the spike times
		nrn->apply_visitor(*this);

		if (spike_time_list_pointer() != 0)
		{
			// go through the spike list
			for (std::deque<Time>::const_iterator i = spike_time_list().begin();
				 i != spike_time_list().end();
				 ++i)
				// outputs to file only if the spike time is between t_start and t_stop
				if ((*i >= t_start) && (*i < t_stop))
					file_wrapper->write_to_file(*i);
		}
		else std::cout << "error while outputting spike times: void spikelist";

		// reinitialise the visitor (sets the spike_time_list_pointer to 0)
		reset();

		// insert separation between the spike trains
		file_wrapper->insert_separation("end_variable");
	}

	template <class FileWrapper>
	inline void export_connectivity(NeuronInterface * const nrn, FileWrapper * const file_wrapper, int pre_post)
	{
	}
};



//! Computes the spiking rate of a neuron.
/*! The rate (in Hz) is computed for all the spikes between times \b t_start and \b t_stop.
	The method template \link SpikingRateOutputter::operation \endlink depends on the the template parameter \b FileWrapper.
 	Derives from DataRecordNeuronVisitor to be able to retrieve the spike list from neurons.
*/
class SpikingRateOutputter
	: public DataRecordNeuronVisitor
{
	template <class KernelFunction> friend class CovarianceOutputter;
protected:
	//!	Type for initialisation value.
	typedef void * InitValue;
	//! Constructor.
	SpikingRateOutputter(InitValue) {}
public:
	//!	Returns 1, one rate per neuron.
	/*!	Used by a class \b NeuronListHandler (see OutputterImpl) to determine the number of outputs to pass to a \b FileWrapper.
	 */
	unsigned nb_outputs_for_neuron(NeuronInterface * const)
	{
		return 1;
	}

	//!	Calls \link SpikingRateOutputter::compute_rate \endlink and send the results to the \b FileWrapper.
	/*! Method template called according to the real type of FileWrapper.
		\param t_start Start time. Anterior spike times are ignored.
		\param t_stop Stop time. Posterior spike times are ignored.
		\param nrn Neuron to treat.
		\param file_wrapper (of type \b FileWrapper).
	 */
	template <class FileWrapper>
	inline void operation(const Time & t_start, const Time & t_stop
						, NeuronInterface * const nrn
						, FileWrapper * const file_wrapper)
	{
		// to retrieve the list of the spike times
		nrn->apply_visitor(*this);

		if (spike_time_list_pointer() != 0)
			file_wrapper->write_to_file(compute_rate(t_start, t_stop, spike_time_list().begin(), spike_time_list().end()));
		else
			std::cout << "error while outputting spike times: void spikelist";

		// reinitialise the visitor (sets the spike_time_list_pointer to 0)
		reset();
	}
	//!	Returns the time-averaged rate of the spike train.
	/*! \param t_start Start time. Anterior spike times are ignored.
		\param t_stop Stop time. Posterior spike times are ignored.
		\param spike_list Spike list on which to perform the averaging.
	 */

	inline double compute_rate(const Time & t_start, const Time & t_stop
							 , const std::deque<Time>::const_iterator & spike_list_iterator_begin
							 , const std::deque<Time>::const_iterator & spike_list_iterator_end)
	{
		// look for the first spike after t_start (iterator i)
		std::deque<Time>::const_iterator i = spike_list_iterator_begin;
		for ( ; i != spike_list_iterator_end;
			 ++i) 
			if ((*i) >= t_start) break;
		// look for the first spike after t_stop (iterator j)
		std::deque<Time>::const_iterator j = i;
		for ( ; j != spike_list_iterator_end;
			 ++j)
			if ((*j) > t_stop) break;
		// returns the result
		return ((double)(j-i)) / (t_stop - t_start);	
	}

	template <class FileWrapper>
	inline void export_connectivity(NeuronInterface * const nrn, FileWrapper * const file_wrapper, int pre_post)
	{
	}

};



//! Computes the covariance between the two spike trains with the kernel \b KernelFunction.
/*! The kernel must be a functor (const std::deque<Time> &, const std::deque<Time> &, const Time &, const Time &).
	The product of the rates of each spike train is is computed for all the spikes between times \b t_start and \b t_stop.
	The method template \link SpikingRateOutputter::operation \endlink depends on the the template parameter \b FileWrapper.
 */
template <class KernelFunction>
class CovarianceOutputter
{
protected:
	//!	Type for initialisation value.
	typedef void * InitValue;
	//! Constructor.
	CovarianceOutputter(InitValue) : sro_(0) {}
public:
	//!	Returns 1, one correlation coefficient per pair of neurons.
	/*!	Used by a class \b NeuronListHandler (see OutputterImpl) to determine the number of outputs to pass to a \b FileWrapper.
	 */
	unsigned nb_outputs_for_pair(NeuronInterface * const, NeuronInterface * const)
	{
		return 1;
	}

	//!	Performs the time-averaged rate of the spike train.
	/*! Method template called according to the real type of FileWrapper.
		\param t_start Start time. Anterior spike times are ignored.
		\param t_stop Stop time. Posterior spike times are ignored.
		\param spike_list Spike list to perform the operation.
		\param file_wrapper (of type \b FileWrapper).
	 */
	template <class FileWrapper>
	inline void operation(const Time & t_start, const Time & t_stop
						, const std::deque<Time> & spike_list1
						, const std::deque<Time> & spike_list2
						, FileWrapper * const file_wrapper)
	{
		double res = 0.;

		std::deque<Time>::const_iterator i1 = spike_list1.begin();
		for ( ; i1 != spike_list1.end(); ++i1) 
			if ((*i1) >= t_start) break;
		std::deque<Time>::const_iterator i2 = spike_list2.begin();
		for ( ; i2 != spike_list2.end(); ++i2) 
			if ((*i2) >= t_start) break;
		if (i2 == spike_list2.end())
			return file_wrapper->write_to_file(res); // no significant spike in spike_list2

		for ( ; i1 != spike_list1.end(); ++i1)
		{
			if ((*i1) >= t_stop) break;
			for ( ; i2 != spike_list2.begin(); --i2)
			{
				if (i2 != spike_list2.end()) 
					if (((*i2) < (*i1) - KernelFunction::cutoff_left) || ((*i2) < t_start))
					{
						++i2;
						break;
					}
			}
			for ( ; i2 != spike_list2.end(); ++i2)
			{
				if (((*i2) < (*i1) + KernelFunction::cutoff_right) && ((*i2) < t_stop))
					res += kf_(*i1, *i2);
				else
					break;
			}
		}
		return file_wrapper->write_to_file(
			res / (t_stop - t_start) 
			- KernelFunction::mass 
			* sro_.compute_rate(t_start, t_stop, spike_list1.begin(), spike_list1.end()) 
			* sro_.compute_rate(t_start, t_stop, spike_list2.begin(), spike_list2.end()));
	}

private:
	KernelFunction kf_; /*!< Kernel used to compute the cross-correlation. Template parameter (must be a binary functor).*/
	SpikingRateOutputter sro_; /*!< Used by \link CovarianceOutputter::operation \endlink to compute the mean firing rates of both \a spike_list1 and \a spike_list2.*/
};



#endif  //SPIKELISTOPERATIONS_H
