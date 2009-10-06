// Dispatcher.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DISPATCHER_H
#define DISPATCHER_H


//#include <boost/mpl/is_sequence.hpp>
//#include <boost/mpl/has_key.hpp>

//#include <boost/mpl/list.hpp>




//! AbstractFactory.
/*! To eliminate the constructor of NullType and be able to provide specialisations certain type.
 */
template <class ReturnType>
class AbstractFactory
{
public:
    virtual ReturnType * const create() = 0;
};



/*

template <class ReturnType,
        , class List
        , class RealTypeList
        , class ListOfTypes>
    : public AbstractVisitor
    , public Visitor<Type1>
struct MultiDispatcher
{
    // check the types of the templates for compatibility of the structure
    BOOST_MPL_ASSERT(( is_sequence< RealTypeList > ));
    BOOST_MPL_ASSERT(( is_sequence< ListOfTypes > ));
    BOOST_MPL_ASSERT(( is_sequence< RealTypeList > ));

    MultiDispatcher(TypeBase * pt1, TypeBase * pt2, TypeBase * pt3, TypeBase * pt4)
        : pt1_(pt1), pt2_(pt2), pt3_(pt3), pt4_(pt4) {}
//  template
    ReturnType retrieve()
    {
        pt1->apply_vis(*this);
//      MultiDispatcher
    }
    TypeBase * pt1_, * pt2_, * pt3_, * pt4_;
}

*/

#endif // DISPATCHER_H 
