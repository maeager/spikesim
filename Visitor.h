// Visitor.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef VISITOR_H
#define VISITOR_H



//! AbstractVisitor: common interface for all visitors.
/*! It is a common abstract class from which all concrete visitors should derive (cf. Visitor template class).
 */
struct AbstractVisitor {
    virtual ~AbstractVisitor() {}
};



//! Visitor: class template to access information within classes (Type).
/*! The behaviour is defined in a class derived from (one or) several Visitor<Type1>, Visitor<Type2>
    in the definition of visit().
    The derivation is associated with one from AbstractVisitor in the derived classes.
    All the classes that take the place of Type must have an accpet method, with AbstractVisitor in argument.
 */
template <class Type>
class Visitor
{
public:
    virtual void visit(Type &) = 0;
};




#endif // !defined(VISITOR_H)
