// TypeList.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TYPELIST_H
#define TYPELIST_H



//////////////////////////////////////////////////
// Definition of NullType

class NullType
{
	NullType() {}
};


/*
//////////////////////////////////////////////////
// TypeList Defs

template <class T, class U = NullType>
struct TypeList
{
	typedef T head;
	typedef U tail;
};


//////////////////////////////////////////////////
// length

template <class TL>
struct length
{
	enum {
		value = 1,
	};
};

template <class T, class U>
struct length<TypeList<T,U> >
{
	enum {
		value = 1 + length<U>::value,
	};
};



//////////////////////////////////////////////////
// selector

template <bool Test, class T, class U>
struct selector
{
	typedef T result;
};


template <class T, class U>
struct selector<false, T, U>
{
	typedef U result;
};



//////////////////////////////////////////////////
// type comparison

template <class T, class U>
struct is_same
{
	enum {
		value = 0,
	};
};

template <class T>
struct is_same<T,T>
{
	enum {
		value = 1,
	};
};



//////////////////////////////////////////////////
// get
// indexing starts at 0

template <class TL, int Index>
struct get
{
	typedef NullType result;
};


template <class T, class U, int Index>
struct get<TypeList<T, U>, Index>
{
	typedef typename get<U, Index-1>::result result;
};


template <class T>
struct get<T, 0>
{
	typedef T result;
};


template <class T, class U>
struct get<TypeList<T, U>, 0>
{
	typedef T result;
};




// run time version

template <class TL>
struct getrt
{
//	static 
};

/*

//////////////////////////////////////////////////
// find

template <class TL, class T>
struct find
{
	enum {
		index = -1,
	};
};


template <class T, class U>
struct find<TypeList<T, U>, T>
{
	enum {
		index = 0,
	};
};



template <class U, class V, class T>
struct find<TypeList<U, V>, T>
{
private:
	enum {
		temp = find<V, T>::index,
	};
public:
	enum {
		index = temp == -1 ? -1 : 1 + temp,
	};
};




//////////////////////////////////////////////////
// Append

template <class TL, class T>
struct append
{
	enum {
		index = -1,
	};
};


template <class T, class U>
struct append<TypeList<T, U>, T>
{
	enum {
		index = 0,
	};
};



template <class U, class V, class T>
struct append<TypeList<U, V>, T>
{
private:
	enum {
		temp = find<V, T>::index,
	};
public:
	enum {
		index = temp == -1 ? -1 : 1 + temp,
	};
};

*/

#endif // !defined(TYPELIST_H)
