// OutputManager.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef OUTPUTMANAGER_H
#define OUTPUTMANAGER_H

#include <map>
#include <string>
#include <fstream>

#include "StringFormatters.h"
#include "SimulationEnvironment.h"
#ifdef PARALLELSIM
#include "ParNetwork.h"
#else
#include "Network.h"
#endif
#include "ConfigBase.h"
#include "AsciiFileWrapper.h"


//! OutputterBase: abstract base class for 
/*! Defines a method do_operation to be called by the OutputManager.
 */
class OutputterBase
{
	friend class OutputManager;
protected:
	//! Definition of the type Tag.
	/*!	Determines conditions (when? etc.) of the outputting.
	 */
	typedef std::string Tag;
protected:
	//!	Constructor.
	explicit OutputterBase(const Tag & type_tag, const Tag & freq_tag) : type_tag_(type_tag), freq_tag_(freq_tag) {}
	//! Virtual destructor.
	/*!	Should not be called
	 */
	virtual ~OutputterBase() {}
	//!	Open the file for writing.
	virtual void open_file() = 0;
	//!	Add an entire group to the list of neurons to be outputted
	virtual void add_group(Group * const gp) = 0;
	//! Operation to do when called by OutputManager
	virtual void do_operation(const Time & t_start, const Time & t_stop) = 0;
	//! Type tag accessor.
	const Tag & type_tag() {return type_tag_;}
	//! Frequency tag accessor.
	const Tag & freq_tag() {return freq_tag_;}
	//! Export the IDs of the neurons treated by the outputter
	virtual void export_ID() = 0;
private:
	const Tag freq_tag_; // outputting during sim ('during') or at the end ('end')
	const Tag type_tag_; // weight, rate, spike
};



//! OutputterImpl: class template with the concrete outputting mechanisms.
/*! Derives from OutputterBase.\n
	The template parameter \b FileWrapper encapsulates the output file management, via methods open, write and close.\n
	The template parameter \b OutputOperation defines a static method operation (NB: the arity can change, it must be compatible with the code in NeuronListHandler).\n
	The template parameter \b NeuronListHandler has an iterator to go across the list(s) of the neurons to treat (can be couples of neurons), via a method iterate.\n
 */
template <class FileWrapper
		, class OutputOperation
		, class NeuronListHandler>
class OutputterImpl
	: public OutputterBase
	, public FileWrapper
	, public OutputOperation
	, public NeuronListHandler
{
public:
	//! Constructor.
	/*! Passes the configurator to all of the mother classes \b FileWrapper, \b OutputOperation and \b NeuronListHandler.
		The types of the initialisation values are defined in each class (FileWrapper, OutputOperation and NeuronListHandler).
		Call FileWrapper to open the output file(s) and prepare the writing in.
		The \b InitValue types defined in \b FileWrapper, \b OutputOperation and \b NeuronListHandler must be copyable.
	 */
	OutputterImpl(const Tag & type_tag, const Tag & freq_tag
				, const typename FileWrapper::InitValue & init_1
				, const typename OutputOperation::InitValue & init_2
				, const typename NeuronListHandler::InitValue & init_3)
		: FileWrapper(init_1)
		, OutputOperation(init_2)
		, NeuronListHandler(init_3)
		, OutputterBase(type_tag, freq_tag)
	{
	}

	//! Open the file via FileWrapper.
	inline void open_file()
	{
		NeuronListHandler::send_info_to_file_wrapper(this, this);
		FileWrapper::open_file();
	}

	//! Destructor.
	/*! Call FileWrapper to close the file(s).
	 */
	~OutputterImpl()
	{
		FileWrapper::close_file();
	}

protected:
	//!	Add an entire group to the list of neurons to be outputted
	void add_group(Group * const gp)
	{
		NeuronListHandler::add_group(gp);
	}

	//! Operation to do when called by OutputManager
	void do_operation(const Time & t_start, const Time & t_stop)
	{
		NeuronListHandler::template iterate<OutputOperation, FileWrapper>(t_start, t_stop, this, this);
	}

	//! Export the IDs of the neurons treated by the outputter 
	void export_ID()
	{
		NeuronListHandler::template export_ID<OutputOperation, FileWrapper>(this, this);
	}
};


// OutputManager class definition
// with static members only
// encapsulates all the data for the simulation, such as the time step, the duration, etc.


//! OutputManager: class definition
/*! Encapsulates the file and filename, the operation and the list of the goups to apply the operation on.
	Its static members 
 */
class OutputManager
{
// initialisation and destruction
public:
	static void set(std::ifstream & is);
	static void add_group_to_outputter(const Size & outputter_id, Group * const gp);
private:
	static void add_outputter(const std::string & key, const std::string & freq_tag, const std::string & output_file_type = "ascii");
	static std::string make_name(const std::string & name_base) {return name_base + StringFormatter::IntToStr(outputter_count_++);}

// accessors to members
public:
	static inline unsigned & i_outputting_period() {return i_outputting_period_;}
	static inline const Time & outputting_period() {return outputting_period_;}

// calls for writing in files and update functions
public:
	//! Opens all the file for all the objects Outputter in outputter_list_.
	static void open_files();
	static void close_all();
	static void do_output(const std::string & freq_tag);
	static void clear_past_of_spike_lists(
#ifdef PARALLELSIM 
	ParNetwork & net);
#else
	Network & net);
#endif
	static void do_output_connectivity(const std::list<Size> & list_nb_pre_nrn, const std::list<Size> & list_nb_post_nrn, int bide);

// members
private:	
	//! Time period (in timesteps) between performing the outputting.
	static unsigned i_outputting_period_;
	//! Time period (in seconds) between performing the outputting.
	static Time outputting_period_; 
	//! Model of name (prefix) to be added to all output file names.
	static std::string name_model_;
	//! Number of outputters. Used for naming the output files.
	static Size outputter_count_;
	//! If true, the "old" past spike history of the neurons (spike lists, etc.) is cleared after the outputting.
	static bool clear_spike_lists_;
	//! List (STL map) of the outputters.
	typedef std::vector<OutputterBase *> OutputterListType;
	static OutputterListType outputter_list_;
//	typedef std::map<std::string, OutputterBase * const> OutputterListType;
	//! Ascci file to output the connectivity of the constructed network
	static AsciiFileWrapper connectivity_output_;
	//! Number of connectivity outputting.
	static Size connect_count_;
	//! If true, the connectivity is outputted when \b do_output_connectivity is called by \b do_connect in the class \b Group.
	static bool is_connectivity_outputted_;
};







#endif // !defined(OUTPUTMANAGER_H
