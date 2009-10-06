// OutputManager.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "math.h"

#include "OutputManager.h"

// file handlers
#include "AsciiFileWrapper.h"
// #include "MatlabFileWrapper.h"

// operations
#include "SpikeListOperations.h"
#include "WeightOperations.h"

// outputted neuron list handlers
#include "OutputGroupHandlers.h"

#include "STDPFunction.h"
#include "Macros.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// OutputManager function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
// static members
Time OutputManager::outputting_period_ = 1.;
unsigned OutputManager::i_outputting_period_ = unsigned(OutputManager::outputting_period_ / SimEnv::timestep());
std::string OutputManager::name_model_ = "";
Size OutputManager::outputter_count_ = 0;
bool OutputManager::clear_spike_lists_ = true;
OutputManager::OutputterListType OutputManager::outputter_list_;
AsciiFileWrapper OutputManager::connectivity_output_(AsciiFileInitialiser("connectivite", 6));
Size OutputManager::connect_count_ = 0;
bool OutputManager::is_connectivity_outputted_ = false;

/////////////////////////////////////////////////
// OutputManager initialisation
void OutputManager::set(std::ifstream & is)
{
    std::string test;

    // get the time period of the outputting
    READ_FROM_FILE(is, outputting_period_, "outputting_period", "OutputManager")
    if (outputting_period_ <= 0) throw ConfigError("OutputManager: outputting_period should be >0");
    i_outputting_period_ = (unsigned)(ceil(outputting_period_ / SimEnv::timestep()));

    // clear past of neuron spike_lists
    READ_FROM_FILE(is, clear_spike_lists_, "clear_spike_lists", "OutputManager")

    // initialise the output files
    READ_FROM_FILE(is, name_model_, "name_model", "OutputManager")
    if (name_model_ == "NULL") name_model_ = "";

    // open the output file for the connectivity
    READ_FROM_FILE(is, is_connectivity_outputted_, "connectivity_outputted", "OutputManager")
    connectivity_output_.open_file();

    // creation of outputters according to script
    if (is.eof())
        throw ConfigError("OutputManager: unexpected end of file, expected 'CREATE_OUTPUTTER' or 'END_SET_OUTPUTS'");
    is >> test;
    while (test != "END_SET_OUTPUTS") {
        if (test == "CREATE_OUTPUTTER") {
            // add an outputter to the manager
            if (is.eof())
                throw ConfigError("OutputManager: unexpected end of file, expected the 1st value for the freq_tag 'CREATE_OUTPUTTER'");
            std::string key;
            is >> key;
            if (is.eof())
                throw ConfigError("OutputManager: unexpected end of file, expected the 2nd value for the freq_tag 'CREATE_OUTPUTTER'");
            std::string freq_tag;
            is >> freq_tag;
            if (is.eof())
                throw ConfigError("OutputManager: unexpected end of file, expected the 3rd value for the freq_tag 'CREATE_OUTPUTTER'");
            std::string output_file_type;
            is >> output_file_type;
            add_outputter(key, freq_tag, output_file_type);
        } else throw ConfigError("OutputManager: expected 'CREATE_OUTPUTTER' or 'END_SET_OUTPUTS', got '" + test + "'");

        // get next optional feature
        if (is.eof())
            throw ConfigError("OutputManager: unexpected end of file, expected 'CREATE_OUTPUTTER' or 'END_SET_OUTPUTS'");
        else
            is >> test;
    }
}

/////////////////////////////////////////////////
// add an output (check no duplicate, sorted by the key in argument)
void OutputManager::add_outputter(const std::string & key, const std::string & freq_tag, const std::string & output_file_type)
{
    // pointer to a new outputter
    OutputterBase * outputter;

    // check freq_tag
    if (freq_tag != "during_sim" && freq_tag != "end_sim" && freq_tag != "each_time_step")
        throw ConfigError("OutputManager: unknown freq_tag '" + freq_tag + "' (should be: 'during_sim', 'end_sim' or 'each_time_step')");

    // choice of outputter
    if (output_file_type == "ascii") {
        std::string name_tmp = make_name(name_model_ + key);
        if (key == "spike") {
            AsciiFileInitialiser afi(name_tmp, (unsigned)log10((double)SimEnv::i_duration()) + 1);
            outputter = new OutputterImpl<AsciiFileWrapper, IndivSpikeOutputter, SingleWayThroughGroups>(key, freq_tag, afi, 0, 0);
        } else if (key == "rate") {
            AsciiFileInitialiser afi(name_tmp, 5);
            outputter = new OutputterImpl<AsciiFileWrapper, SpikingRateOutputter, SingleWayThroughGroups>(key, freq_tag, afi, 0, 0);
        }
        /*      else if (key == "correl")
                {
                    AsciiFileInitialiser afi(name_tmp, 5);
                    outputter = new OutputterImpl<AsciiFileWrapper, CovarianceOutputter<STDPFunction>, AllCrossPairsThroughSameGroups>(key, freq_tag, afi, 0, 0);
                }
        */      else if (key == "weight") {
            AsciiFileInitialiser afi(name_tmp, 7);
            outputter = new OutputterImpl<AsciiFileWrapper, WeightOutputter, SingleWayThroughGroups>(key, freq_tag, afi, 0, 0);
        } else throw ConfigError("OutputManager: unknown key '" + key + "' (should be: 'weight', 'rate', 'correl' or 'spike')");
    }
    /*  else if (output_file_type == "matlab")
        {
            std::string var_name_tmp = make_name(name_model_ + key);
            // send the number of outputting, to create matlab matrices end cells of suitable dimensions
            unsigned nb_outputting_iterations = 0;
            if (freq_tag == "during_sim")
                nb_outputting_iterations = (unsigned) ceil((double) SimEnv::i_duration() / i_outputting_period());
            else if (freq_tag == "end_sim")
                nb_outputting_iterations = 1;
            else if (freq_tag == "each_time_step")
                nb_outputting_iterations = SimEnv::i_duration();

            // creates the outputter
            if (key == "spike")
            {
                MatlabFileInitialiser mfi(name_model_, var_name_tmp, nb_outputting_iterations);
                outputter = new OutputterImpl<MatlabFileWrapper<SpikeListToMatlabCellArray>, IndivSpikeOutputter, SingleWayThroughGroups>(key, freq_tag, mfi, 0, 0);
            }
            else if (key == "rate")
            {
                MatlabFileInitialiser mfi(name_model_, var_name_tmp, nb_outputting_iterations);
                outputter = new OutputterImpl<MatlabFileWrapper<ToMatlabMatrix>, SpikingRateOutputter, SingleWayThroughGroups>(key, freq_tag, mfi, 0, 0);
            }
    / *     else if (key == "correl")
            {
                MatlabFileInitialiser mfi(name_model_, var_name_tmp, nb_outputting_iterations);
                outputter = new OutputterImpl<MatlabFileWrapper<ToMatlabMatrix>, CovarianceOutputter<STDPFunction>, AllCrossPairsThroughSameGroups>(key, freq_tag, mfi, 0, 0);
            }
    * /     else if (key == "weight")
            {
                MatlabFileInitialiser mfi(name_model_, var_name_tmp, nb_outputting_iterations);
                outputter = new OutputterImpl<MatlabFileWrapper<ToMatlabMatrix>, WeightOutputter, SingleWayThroughGroups>(key, freq_tag, mfi, 0, 0);
            }
            else throw ConfigError("OutputManager: unknown key '" + key + "' (should be: 'weight', 'rate', 'correl' or 'spike')");
        }
    */  else throw ConfigError("OutputManager: unknown type of output file (should be: 'ascii' or 'matlab'");

    // add the newly created outputter to the list
    outputter_list_.push_back(outputter);
}

/////////////////////////////////////////////////
// add a list of neurons or an entire group to output
void OutputManager::add_group_to_outputter(const Size & outputter_id, Group * const gp)
{
    if (outputter_id < outputter_list_.size())
        outputter_list_[outputter_id]->add_group(gp);
    else
        throw ConfigError("OutputManager: outputter id number out of range");
}

//! Open all the files
void OutputManager::open_files()
{
    for (OutputterListType::iterator i = outputter_list_.begin();
            i != outputter_list_.end();
            ++i)
        (*i)->open_file();
}

//! Open all the files
void OutputManager::close_all()
{
    // close the output file for the connectivity
    connectivity_output_.close_file();
    // clear the memory (the destructors will close the files)
    for (OutputterListType::iterator i = outputter_list_.begin();
            i != outputter_list_.end();
            ++i)
        delete(*i);
}

/////////////////////////////////////////////////
// performs the outputting of all the outputters whose freq_tag is the same as the freq_tag argument
void OutputManager::do_output(const std::string & freq_tag)
{
    for (OutputterListType::iterator i = outputter_list_.begin();
            i != outputter_list_.end();
            ++i)
        if ((*i)->freq_tag() == freq_tag)
            (*i)->do_operation(SimEnv::sim_time() - outputting_period(), SimEnv::sim_time());
}

/////////////////////////////////////////////////
// performs the outputting during the simulation
#ifdef PARALLELSIM
void OutputManager::clear_past_of_spike_lists(ParNetwork & net)
#else
void OutputManager::clear_past_of_spike_lists(Network & net)
#endif
{
    if (clear_spike_lists_)
        net.clear_past_of_spike_list(SimEnv::sim_time() - outputting_period());
}

/////////////////////////////////////////////////
// performs the outputting during the simulation
void OutputManager::do_output_connectivity(const std::list<Size> & list_nb_pre_nrn, const std::list<Size> & list_nb_post_nrn, int bide)
{
    if (bide == 0) {
        if (is_connectivity_outputted_) {
            connectivity_output_.write_to_file(std::string("% connection batch ") + StringFormatter::IntToStr(connect_count_));
            connectivity_output_.insert_separation("");
            connectivity_output_.write_to_file(std::string("v_pre_") + StringFormatter::IntToStr(connect_count_) + std::string(" = ["));
            for (std::list<Size>::const_iterator i = list_nb_pre_nrn.begin();
                    i != list_nb_pre_nrn.end();
                    ++i)
                connectivity_output_.write_to_file(*i);
            connectivity_output_.write_to_file(std::string("];"));
            connectivity_output_.insert_separation("");
            connectivity_output_.write_to_file(std::string("v_post_") + StringFormatter::IntToStr(connect_count_) + std::string(" = ["));
            for (std::list<Size>::const_iterator i = list_nb_post_nrn.begin();
                    i != list_nb_post_nrn.end();
                    ++i)
                connectivity_output_.write_to_file(*i);
            connectivity_output_.write_to_file(std::string("];"));
            connectivity_output_.insert_separation("");
            ++connect_count_;
        }
    } else {
        // verif export connect
        for (OutputterListType::iterator i = outputter_list_.begin();
                i != outputter_list_.end();
                ++i)
            if ((*i)->type_tag() == "weight")
                (*i)->export_ID();
    }
}
