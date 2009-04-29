// Error.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <iostream>

//!	Run-time error class.
/*!	Enncapsulation of the STL exception system.
 */
class Error
	: public std::exception
{
public:
	//! Constructor.
	explicit Error(const std::string msg = "") : std::exception(), msg_(msg) {}
	~Error() throw() {}
	const char * what() const throw() {return msg_.c_str();}
private:
    std::string msg_;
};


//!	Run-time error during the configuration process.
/*!	Derives from Error.
 */
class ConfigError
	: public Error
{
public:
	//! Constructor.
	explicit ConfigError(const std::string msg = "") : Error(msg) {}
};

#endif // !ERROR_H
