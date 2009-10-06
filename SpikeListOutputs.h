// SpikeListOutputs.h
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef SPIKELISTOUTPUTS_H
#define SPIKELISTOUTPUTS_H


#include <string>

#include <mat.h>

#include "OutputManager.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
// definition of IndivSpikeOutputter class
// outputs the spikes individually to a file
////////////////////////////////////////////////////////////////////////////////////////////////////

class IndivSpikeOutputter
        : public Outputter
{
public:
    IndivSpikeOutputter() {}
protected:
    void do_operation(const Time & , const Time &) {
        // one file per group
        pmat_ = matOpen(file_name().c_str(), "w");
        matClose(pmat_);

        DataRecordNeuronVisitor vis;
        for (std::list<Group * const>::const_iterator i = group_list().begin();
                i != group_list().end();
                ++i) {
            std::string varname;
            char tmp[3];
            _itoa_s((*i)->id(), tmp, 3, 10);
            varname = std::string("Group") + tmp;
            std::cout << "writing spikes for " << varname << std::endl;

            Size numSpikeVecs = (Size)(*i)->size();

            mxArray * mxArr = 0
                              , * mxCell = mxCreateCellMatrix((mwSize)numSpikeVecs, 1);
            double * data = 0;
            //-------
            mwIndex jstruct = 0;
            Size count = 1;

            for (std::list<NeuronInterfaceBase * const>::const_iterator j = get_neuron_list(*i).begin();
                    j != get_neuron_list(*i).end();
                    ++j) {
                (*j)->apply_visitor(vis);

                if (vis.spike_time_list_pointer() != 0) {
                    // init the array where to put the spike times
                    if (mxArr) mxDestroyArray(mxArr);
                    mxArr = mxCreateDoubleMatrix(1, (Size) vis.spike_time_list_pointer()->size(), mxREAL);
                    data = mxGetPr(mxArr);
                    // test if initialisation went well
                    if (!data) {
                        // not enough memory suppposedly
                        mxDestroyArray(mxCell);
                        mxDestroyArray(mxArr);
                        return;
                        std::cout << "prob when writing spike times for " << varname << std::endl;
                    }

                    // write the spike times in the array of doubles
                    Size pos = 0;
                    for (std::deque<Time>::const_iterator k = vis.spike_time_list_pointer()->begin();
                            k != vis.spike_time_list_pointer()->end();
                            ++k, ++pos) {
//                  file_ << (*k) << "\t";
                        data[pos] = (*k);
                    }

                    // append the array in the cell structure
                    mxSetCell(mxCell, jstruct++, mxDuplicateArray(mxArr));

                } else std::cout << "prob: empty spikelist";
                vis.reset();
            }

            // write the cell structure in the file
            pmat_ = matOpen(file_name().c_str(), "u");
            if (pmat_) {
                //Write the Cell Array to the MATfile

                int retFlag = matPutVariable(pmat_, varname.c_str(), mxCell);
                matClose(pmat_);
                //Free memory
                mxDestroyArray(mxCell);

                if (retFlag != 0) {
                    std::cout << "error writing spike trains in matlab file: " + retFlag;
                }
            }

        }
    }
private:
    MATFile * pmat_;
};



////////////////////////////////////////////////////////////////////////////////////////////////////
// definition of SingleSpikeListOutputter class
// computes the operation on each spike list
// Operation is a functor (const std::deque<Time> &, const Time &, const Time &)
////////////////////////////////////////////////////////////////////////////////////////////////////

template <class Operation>
class SingleSpikeListOperationOutputter
        : public Outputter
{
protected:
    void do_operation(const Time & t_start, const Time & t_stop) {
        DataRecordNeuronVisitor vis;
        for (std::list<Group * const>::const_iterator i = group_list().begin();
                i != group_list().end();
                ++i)
            for (std::list<NeuronInterfaceBase * const>::const_iterator j = get_neuron_list(*i).begin();
                    j != get_neuron_list(*i).end();
                    ++j) {
                (*j)->apply_visitor(vis);
                file_ << op(vis.spike_time_list(), t_start, t_stop) << "\t";
                vis.reset();
            }
        file_ << std::endl;
    }
private:
    Operation op;
};



////////////////////////////////////////////////////////////////////////////////////////////////////
// definition of CrossSpikeListOperationOutputter class
// computes the operation on each spike list
// Operation is a functor (const std::deque<Time> &, const std::deque<Time> &, const Time &, const Time &)
////////////////////////////////////////////////////////////////////////////////////////////////////

template <class Operation>
class CrossSpikeListOperationOutputter
        : public Outputter
{
protected:
    void do_operation(const Time & t_start, const Time & t_stop) {
        DataRecordNeuronVisitor vis1, vis2;
        for (std::list<Group * const>::const_iterator i1 = group_list().begin();
                i1 != group_list().end();
                ++i1)
            for (std::list<NeuronInterfaceBase * const>::const_iterator j1 = get_neuron_list(*i1).begin();
                    j1 != get_neuron_list(*i1).end();
                    ++j1) {
                (*j1)->apply_visitor(vis1);
                for (std::list<Group * const>::const_iterator i2 = group_list().begin();
                        i2 != group_list().end();
                        ++i2)
                    for (std::list<NeuronInterfaceBase * const>::const_iterator j2 = get_neuron_list(*i2).begin();
                            j2 != get_neuron_list(*i2).end();
                            ++j2) {
                        (*j2)->apply_visitor(vis2);
                        file_ << op_(vis1.spike_time_list(), vis2.spike_time_list(), t_start, t_stop) << "\t";
                        vis2.reset();
                    }
                vis1.reset();
                file_ << std::endl;
            }
        file_ << std::endl;
    }
private:
    Operation op_;
};




#endif  //SPIKELISTOUTPUTS_H
