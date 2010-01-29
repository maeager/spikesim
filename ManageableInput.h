// ManageableInput.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MANAGEABLEINPUT_H
#define MANAGEABLEINPUT_H


#include <list>

#include "Error.h"
#include "GlobalDefs.h"


class ManageableInput;


//! ManageableInputManager class definition to centralise all manageable inputs and launch the general update
class ManageableInputManager
{
    friend class ManageableInput;
    friend class Threading;
public:
    static void input_update_general();
//  static ManageableInput * const get_primary_input(unsigned i);
protected:
    static void append(ManageableInput * const pmi);
    static std::list<ManageableInput *> list_inputs_primary_;
//  static std::vector<ManageableInput *> list_inputs_secondary_;
};



//! ManageableInput class definition
/*! Base class for any input that has to be updated prior to any other mechanism at each simulation time step
 */
class ManageableInput
{
    friend class ManageableInputManager;
    friend class Threading;
protected:
    ManageableInput(bool is_primary = true) {
        if (is_primary)
            ManageableInputManager::append(this);
//      else
//          ManageableInputManager::append(this);
    }
    virtual void input_update() = 0;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// inline definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/*! call the update method for all the manageable inputs
 * primary vector first, then secondary vector (some of the primary may be used by some in secondary)
 */
inline void ManageableInputManager::input_update_general()
{
    for (std::list<ManageableInput *>::const_iterator i = list_inputs_primary_.begin();
            i != list_inputs_primary_.end();
            ++i)
        (*i)->input_update();


    //for (std::vector<ManageableInput *>::iterator i = list_inputs_primary_.begin();
    //   i != list_inputs_primary_.end();
    //   ++i)
    //  (*i)->input_update();
    //for (std::vector<ManageableInput *>::iterator i = list_inputs_secondary_.begin();
    //   i != list_inputs_secondary_.end();
    //   ++i)
    //  (*i)->input_update();
}
/*
/////////////////////////////////////////////////
// return the ith primary input of the list
inline ManageableInput * const ManageableInputManager::get_primary_input(unsigned i)
{
    if (i <= list_inputs_primary_.size())
        return list_inputs_primary_[i];
    else
        throw ConfigError("ManageableInput: i out of bound, the input sought for does not exist");
}
*/

#endif // !defined(MANAGEABLEINPUT_H)
