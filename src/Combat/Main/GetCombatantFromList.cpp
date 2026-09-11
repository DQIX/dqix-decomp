#include <globaldefs.h>
#include "GameState.h"

GameObject* GameState::GetCombatantByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    if (objects_[idx] == NULL)
        return NULL;
    if (!(objects_[idx]->obj3D_.unknown_0_ & 0x80))
        return NULL;
    return objects_[idx];
}