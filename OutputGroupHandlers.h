// OutputGroupHandlers.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef OUTPUTTEDNEURONHANDLERS_H
#define OUTPUTTEDNEURONHANDLERS_H

#include <list>
#include <deque>

#include "GlobalDefs.h"


//! Has a list of groups (does not own them), treats each neuron within each group, group after group.
/*! This class has a protected constructor, and shall only be called by OutputterImpl.
    The method iterate is called by the OutputManager, which calls the operation passed in operation_function then.
    Derives from DataRecordNeuronVisitor to be able to retrieve the spike list from neurons.
    It is a friend of the Group class.
 */
class SingleWayThroughGroups
{
protected:
    //! Type of initialisation value.
    typedef void * InitValue;
    //! Constructor.
    SingleWayThroughGroups(InitValue) {}

    //! Add a group to the list.
    void add_group(Group * const gp) {
        group_list_.push_back(gp);
    }

    //! Go through the list of groups to output the IDs of the neurons or of the connections (pre- and post-neurons)
    template <class OutputOperation, class FileWrapper>
    void export_ID(OutputOperation * const output_operation, FileWrapper * const file_wrapper);

    //! Sends information about the number of neurons in all the groups to \b file_wrapper via \b output_operation.
    /*! Template function to be called according to the real types of OutputOperation and FileWrapper.
     */
    template <class OutputOperation, class FileWrapper>
    void send_info_to_file_wrapper(OutputOperation * const output_operation, FileWrapper * const file_wrapper);

    //! Operation called by OutputManager: iterates over the neurons to treat and call the operation.
    /*! Template function to be called according to the real types of OutputOperation and FileWrapper.
     */
    template <class OutputOperation, class FileWrapper>
    void iterate(const Time & t_start, const Time & t_stop
                 , OutputOperation * const output_operation
                 , FileWrapper * const file_wrapper);

private:
    //! list of groups
    typedef std::list<Group *> GroupListType;
    GroupListType group_list_; // this class does not own the groups
};



//! Holds a list of groups, and treats all pairs of neurons of each group (including self-pairs).
/*! This class has a protected constructor, and shall only be called by OutputterImpl.
    The method iterate is called by the OutputManager, which calls the operation passed in operation_function then.
    It is a friend of the Group class.
 */
class AllCrossPairsThroughSameGroups
{
protected:
    //! Type of initialisation value.
    typedef void * InitValue;
    //! Constructor.
    AllCrossPairsThroughSameGroups(InitValue) {}

    //! Add a group to the list.
    void add_group(Group * const gp) {
        group_list_.push_back(gp);
    }

    //! Do nothing - called for SingleWayThroughGroups so far 
    template <class OutputOperation, class FileWrapper>
    void export_ID(OutputOperation * const output_operation, FileWrapper * const file_wrapper) {
    }

    //! Sends information about the number of neurons in all the groups to \b file_wrapper via \b output_operation.
    /*! Template function to be called according to the real types of OutputOperation and FileWrapper.
     */
    template <class OutputOperation, class FileWrapper>
    void send_info_to_file_wrapper(OutputOperation * const output_operation, FileWrapper * const file_wrapper);

    //! Operation called by OutputManager: iterates over the neurons to treat and call the operation.
    /*! Template function to be called according to the real types of OutputOperation and FileWrapper.
     */
    template <class OutputOperation, class FileWrapper>
    void iterate(const Time & t_start, const Time & t_stop
                 , OutputOperation * const output_operation
                 , FileWrapper * const file_wrapper);

private:
    //! list of groups
    typedef std::list<Group *> GroupListType;
    GroupListType group_list_; // this class does not own the groups
};


//////////////////////////////////////////////////////////////////////////////////////////////////
// SingleWayThroughGroups inline definitions
//////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// method that treat each neuron of all the groups (group by group, can be twice treatment if redondant between groups)
template <class OutputOperation, class FileWrapper>
void SingleWayThroughGroups::iterate(const Time & t_start, const Time & t_stop
                                     , OutputOperation * const output_operation
                                     , FileWrapper * const file_wrapper)
{
    for (GroupListType::const_iterator i = group_list_.begin();
            i != group_list_.end();
            ++i) {
        for (Group::ListNrnType::const_iterator j = (*i)->list_.begin();
                j != (*i)->list_.end();
                ++j)
            // call the operation function to treat the spike time list
            output_operation->template operation<FileWrapper>(t_start, t_stop, (*j).operator->(), file_wrapper);
    }

    // separation
    file_wrapper->insert_separation("end_output");
}

/////////////////////////////////////////////////
// Go through the list of groups to output the IDs of the neurons or of the connections (pre- and post-neurons)
template <class OutputOperation, class FileWrapper>
void SingleWayThroughGroups::export_ID(OutputOperation * const output_operation
                                       , FileWrapper * const file_wrapper)
{
    for (GroupListType::const_iterator i = group_list_.begin();
            i != group_list_.end();
            ++i) {
        for (Group::ListNrnType::const_iterator j = (*i)->list_.begin();
                j != (*i)->list_.end();
                ++j)
            // call the operation function to treat the spike time list
            output_operation->export_connectivity((*j).operator->(), file_wrapper, 0);
    }
    file_wrapper->insert_separation("end_output");
    for (GroupListType::const_iterator i = group_list_.begin();
            i != group_list_.end();
            ++i) {
        for (Group::ListNrnType::const_iterator j = (*i)->list_.begin();
                j != (*i)->list_.end();
                ++j)
            // call the operation function to treat the spike time list
            output_operation->export_connectivity((*j).operator->(), file_wrapper, 1);
    }
    file_wrapper->insert_separation("end_output");
}

/////////////////////////////////////////////////
// send the number of neurons in all the groups to file_wrapper
template <class OutputOperation, class FileWrapper>
void SingleWayThroughGroups::send_info_to_file_wrapper(OutputOperation * const output_operation, FileWrapper * const file_wrapper)
{
    Size count = 0;
    for (GroupListType::const_iterator i = group_list_.begin();
            i != group_list_.end();
            ++i)
        for (Group::ListNrnType::const_iterator j = (*i)->list_.begin();
                j != (*i)->list_.end();
                ++j)
            count += output_operation->nb_outputs_for_neuron((*j).operator->());
    file_wrapper->info_from_neuron_handler(count);
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// AllCrossPairsThroughSameGroups inline definitions
//////////////////////////////////////////////////////////////////////////////////////////////////

//FixMe need != operator for i1 j1 i2 j2
//bool operator!=(CONST std::list<NeuronInterface*>::const_iterator& itRHS)
//{return(!(*this == itRHS));}


/////////////////////////////////////////////////
// method that treat each pair of neurons over all the groups
// group after group, can be twice treatment if redondant between groups
// including self-pairs
template <class OutputOperation, class FileWrapper>
void AllCrossPairsThroughSameGroups::iterate(const Time & t_start, const Time & t_stop
        , OutputOperation * const output_operation
        , FileWrapper * const file_wrapper)
{
    DataRecordNeuronVisitor vis1, vis2;
    for (std::list<Group *>::const_iterator i1 = group_list_.begin();
            i1 != group_list_.end();
            ++i1)
        for (std::list<NeuronInterface *>::const_iterator j1 = (*i1)->list_.begin();
                j1 != (*i1)->list_.end();
                ++j1) {
            (*j1)->apply_visitor(vis1);
            for (std::list<Group *>::const_iterator i2 = group_list_.begin();
                    i2 != group_list_.end();
                    ++i2)
                for (std::list<NeuronInterface *>::const_iterator j2 = (*i2)->list_.begin();
                        j2 != (*i2)->list_.end();
                        ++j2) {
                    (*j2)->apply_visitor(vis2);
                    output_operation->template operation<FileWrapper>(t_start, t_stop, vis1.spike_time_list(), vis2.spike_time_list(), file_wrapper);
                    vis2.reset();
                }
            vis1.reset();
        }

    // separation
    file_wrapper->insert_separation("end_output");
}


/////////////////////////////////////////////////
// send the number of neurons in all the groups (twice) to file_wrapper
template <class OutputOperation, class FileWrapper>
void AllCrossPairsThroughSameGroups::send_info_to_file_wrapper(OutputOperation * const output_operation, FileWrapper * const file_wrapper)
{
    Size count = 0;
    for (std::list<Group *>::const_iterator i1 = group_list_.begin();
            i1 != group_list_.end();
            ++i1)
        for (std::list<NeuronInterface *>::const_iterator j1 = (*i1)->list_.begin();
                j1 != (*i1)->list_.end();
                ++j1) {
            for (std::list<Group *>::const_iterator i2 = group_list_.begin();
                    i2 != group_list_.end();
                    ++i2)
                for (std::list<NeuronInterface *>::const_iterator j2 = (*i2)->list_.begin();
                        j2 != (*i2)->list_.end();
                        ++j2)
                    count += output_operation->nb_outputs_for_pair(*j1, *j2);
        }
    file_wrapper->info_from_neuron_handler(count);
}

#endif // OUTPUTTEDNEURONHANDLERS_H
