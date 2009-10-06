// WeightOutputs.h
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef WEIGHTOUTPUTS_H
#define WEIGHTOUTPUTS_H


#include "OutputManager.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
// definition of WeightOutputter class
// computes the operation on each spike list
// Operation is a functor (const std::deque<Time> &, const Time &, const Time &)
////////////////////////////////////////////////////////////////////////////////////////////////////

class WeightOutputter
        : public Outputter
{
protected:
    void do_operation(const Time & t_start, const Time & t_stop) {
        DataCommonNeuronVisitor vis;
        for (std::list<Group * const>::const_iterator i = group_list().begin();
                i != group_list().end();
                ++i)
            for (std::list<NeuronInterfaceBase * const>::const_iterator j = get_neuron_list(*i).begin();
                    j != get_neuron_list(*i).end();
                    ++j) {
                (*j)->apply_visitor(vis);
                for (std::list<SynapseInterfaceBase * const>::const_iterator k = vis.postsyn_list_pointer()->begin();
                        k != vis.postsyn_list_pointer()->end();
                        ++k)
                    file_ << (*k)->weight() << "\t";
                vis.reset();
            }
        file_ << std::endl;
    }
};



#endif  //WEIGHTOUTPUTS_H