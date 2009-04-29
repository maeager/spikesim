// StringFormatter.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef STRINGFORMATTER_H
#define STRINGFORMATTER_H

#include <sstream>
//#include <boost/format.hpp>


struct StringFormatter
{
	//! Converts an unsigned integer to a string.
	template <class T>
	static std::string IntToStr(const T & num)
	{
		std::stringstream temp;
		temp << num;
		return temp.str();
//		return (boost::format("%d") % num).str();
	}
};



#endif // !STRINGFORMATTER_H
