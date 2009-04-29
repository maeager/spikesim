// ConfigBase.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CONFIGBASE_H
#define CONFIGBASE_H


#include "Visitor.h"


//! ConfigBase: common interface for all configurators.
/*!	Abstract class. It is visitable (method apply_vis to define in all the derived classes) and non copyable.
 */
class ConfigBase
{
public:
	//! Destructor.
	virtual ~ConfigBase() {}
	//! Accept method for visitors (cf. AbstractVisitor and Visitor).
	virtual void apply_vis(AbstractVisitor &) = 0;
protected:
	//! Constructor.
	/*! Protected constructor, can only be called by derived classes.
	 */
	ConfigBase() {}
private:
	//! To make this class non copyable.
	ConfigBase(const ConfigBase &) {}
};



#endif // !defined(CONFIGBASE_H)
