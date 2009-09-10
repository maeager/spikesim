// ManageableInput.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "ManageableInput.h"


std::list<ManageableInput *> ManageableInputManager::list_inputs_primary_;
//std::vector<ManageableInput *> ManageableInputManager::list_inputs_secondary_;



void ManageableInputManager::append(ManageableInput * const pmi)
{
    list_inputs_primary_.push_back(pmi);
}
