// WeightOperations.h
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef WEIGHTOPERATIONS_H
#define WEIGHTOPERATIONS_H


#include "OutputManager.h"


//! Outputs the weights of the post-synapses of a neuron.
/*! The method template \link SpikingRateOutputter::operation \endlink depends on the the template parameter \b FileWrapper.
 */
class WeightOutputter
        : public DataCommonNeuronVisitor
{
protected:
    //! Type for initialisation value.
    typedef void * InitValue;
    //! Constructor.
    WeightOutputter(InitValue) {}
public:
    //! Returns the number of post_synapses of the neuron.
    /*! Used by a class \b NeuronListHandler (see OutputterImpl) to determine the number of outputs to pass to a \b FileWrapper.
     */
    unsigned nb_outputs_for_neuron(NeuronInterface * const nrn) {
        unsigned count = 0;
        // retrieve the list of post-synapses and counts them
        nrn->apply_visitor(*this);
        if (postsyn_list_pointer())
            for (DataCommonNeuronVisitor::ListPostSynType::const_iterator i = postsyn_list_pointer()->begin();
                    i != postsyn_list_pointer()->end();
                    ++i)
                ++count;
        reset();
        return count;
    }

    //! Performs the outputting of the weights of the post-synaptic spikes of a neuron.
    /*! Method template called according to the real type of FileWrapper.
        \param t_start Useless here.
        \param t_stop Useless here.
        \param nrn Neuron to treat.
        \param file_wrapper (of type \b FileWrapper).
     */
    template <class FileWrapper>
    inline void operation(const Time & t_start, const Time & t_stop
                          , NeuronInterface * const nrn
                          , FileWrapper * const file_wrapper) {
        // to retrieve the list of post-synapses
        nrn->apply_visitor(*this);
        // go through the list of post-synapses
        if (postsyn_list_pointer())
            for (DataCommonNeuronVisitor::ListPostSynType::const_iterator i = postsyn_list_pointer()->begin();
                    i != postsyn_list_pointer()->end();
                    ++i)
                file_wrapper->write_to_file((*i)->weight());
        // reinitialise the visitor (sets the spike_time_list_pointer to 0)
        reset();
    }

    //! Returns the number of post_synapses of the neuron.
    /*! Used by a class \b NeuronListHandler (see OutputterImpl) to export the ID of the post-neurons of \b nrn.
     */
    template <class FileWrapper>
    inline void export_connectivity(NeuronInterface * const nrn, FileWrapper * const file_wrapper, int pre_post) {
        unsigned count = 0;
        // retrieve the id of the post-neurons of post-synapses
        nrn->apply_visitor(*this);
        if (postsyn_list_pointer())
            for (DataCommonNeuronVisitor::ListPostSynType::const_iterator i = postsyn_list_pointer()->begin();
                    i != postsyn_list_pointer()->end();
                    ++i)
                if (pre_post == 0)
                    file_wrapper->write_to_file(nrn->id());
                else
                    file_wrapper->write_to_file((*i)->post_nrn().id());
        reset();
    }
};



#endif  //WEIGHTOPERATIONS_H
