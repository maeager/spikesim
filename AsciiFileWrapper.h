// AsciiFileWrapper.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ASCIIFILEWRAPPER_H
#define ASCIIFILEWRAPPER_H


#include <string>
#include <fstream>

#include "GlobalDefs.h"


//! AsciiFileInitialiser: encapsulates data used by the constructor of AsciiFileWrapper.
class AsciiFileInitialiser
{
	friend class AsciiFileWrapper;
public:
	//! Constructor.
	AsciiFileInitialiser(const std::string & file_name, const Size & precision)
		: file_name_(file_name_formatter(file_name))
		, precision_(precision)
	{}
	//! Copy constructor.
	AsciiFileInitialiser(const AsciiFileInitialiser & afi)
		: file_name_(afi.file_name_)
		, precision_(afi.precision_)
	{}
private:
	static std::string file_name_formatter(const std::string & file_name) {return std::string(file_name + ".txt");}
	const std::string file_name_; /*!< File name. */
	const Size precision_; /*!< Number of digits after the dot when writing floats to the file. */
};



//! AsciiFileWrapper: manages the access to an ASCII file.
/*!	This class has a protected constructor, and shall only be called by OutputterImpl.
	It uses the STL file system and outputs double values in an ascii file, separated with tabulations and return to line characters.
 */
class AsciiFileWrapper
{
	friend class OutputManager;
protected:
	//!	Type of initialisation value.
	typedef AsciiFileInitialiser InitValue;

	//!	Constructor.
	AsciiFileWrapper(InitValue init_param) : init_param_(init_param) {}

	//! Opens the file in writing mode.
	inline void open_file()
	{
		// one file per group
		file_.open(init_param_.file_name_.c_str());
		// initialise the file
		file_.clear();
		file_.fill(0);
		file_.precision(init_param_.precision_);
	}

	//! Close the file.
	inline void close_file()
	{
		// close file
		file_.close();
	}
	
public:
	//! Empty method, not used here.
	void info_from_neuron_handler(const unsigned &) {}

	//! Writes a double to the file and a tabulation after.
	inline void write_to_file(const double & val)
	{
		file_ << val << "\t";
	}

	//! Writes a double to the file and a tabulation after.
	inline void write_to_file(const std::string & str)
	{
		file_ << str << "\t";
	}

	//! Insert a 'return to line' character in the ascii file.
	/*!	Has a void argument.
	 */
	inline void insert_separation(const std::string &)
	{
		file_ << std::endl;
	}

private:
	//!	STL file.
	std::ofstream file_;
	//!	File initialisation object.
	const AsciiFileInitialiser init_param_;
};



#endif  //ASCIIFILEWRAPPER_H
