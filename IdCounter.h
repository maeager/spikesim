// IdCounter.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef IDCOUNTER_H
#define IDCOUNTER_H


#include "GlobalDefs.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// IdCounter class definition
// automatically tags the derived object with an positive integer, which is accessible trhough the method id()
///////////////////////////////////////////////////////////////////////////////////////////////////

template <class CountedType>
class IdCounter
{
public:
    const Size & id() const {
        return id_;
    }
protected:
    IdCounter() : id_(static_counter_++) {}
private:
    static Size static_counter_;
    const Size id_;
};

template <class CountedType> Size IdCounter<CountedType>::static_counter_ = 0;



#endif // !defined(IDCOUNTER_H)
