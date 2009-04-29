// HandleArray.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef HANDLE_H
#define HANDLE_H


#include "GlobalDefs.h"

/*

//!	Constant pointer encapsulation.
template <class T>
class Handle
{
public:
    explicit Handle(T * const pt) : pt_(pt) {}
    Handle(const Handle & h) : pt_(h.pt_) {}
    ~Handle() {}
    int operator == (const Handle & h) const {return (pt_ == h.pt_);}
    T * const operator->() const {return pt_;}
private:
    Handle & operator = (const Handle &);
    T * const pt_;
};

//!	Constant pointer encapsulation.
template <class T>
class HandleArray
{
public:
    HandleArray() {}
    ~HandleArray() {delete[] array;}
    void resize(const Size & size) {if (array_) delete[] array_; array_ = new Handle<T>[size];}
    const Handle<T> & operator[](const Size & i) {return array_[i];}
    
    typedef Handle<T> * iterator;
    static const L = sizeof(Handle<T>);
    const iterator & begin() {return & array[0];}
    const iterator & end() {return & array[size_-1] + L;}
    const iterator & operator++(iterator & it) {return iterator + L;}
private:
    HandleArray(const HandleArray &);
    HandleArray & operator = (const HandleArray &);
    Size size_;
    Handle<T> * array_;
};

*/
#endif // !defined(HANDLE_H)
