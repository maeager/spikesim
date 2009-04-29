// Data.h
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef DATA_H
#define DATA_H

/*
#include "Error.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Definition of Data to deal with data possibly non initialised
////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T> 
class Data
{
public:
	Data() : pt_value(void_value_) {}
	explicit Data(const T & val_init) {alloc(val_init);}
	~Data() {dealloc();}
	Data & operator=(T & t) {set(t); return *this;}
	void set(const T & val) {if (non_init()) alloc(val); else *pt_value = val;}
	const T & value() const {if (non_init()) throw Error("Data: not initialised"); else return *pt_value;}
private:
	bool non_init() const {return (pt_value == void_value_);}
	void alloc(const T & val) {pt_value = new T(val);}
	void dealloc() {if (! non_init()) delete pt_value;}
	T * pt_value;
	static T * const void_value_;
};

template <class T>
T * const Data<T>::void_value_ = new T;


// specialisation to discard using T that are pointer or reference types

template <class T> 
class Data<T *>
{
	Data() {}
};


template <class T> 
class Data<T * const>
{
	Data() {}
};


template <class T, unsigned N> 
class Data<T[N]>
{
	Data() {}
};


template <class T> 
class Data<T &>
{
	Data() {}
};

*/

#endif  //DATA_H
