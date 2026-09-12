#include <globaldefs.h>
#include "GameState.h"

GameObject* GameState::GetGameObjectByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    return objects_[idx];
}

GameObject* GameState::GetMaybeWanderingMonsterByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    if (objects_[idx] == NULL)
        return NULL;
    if (!(objects_[idx]->obj3D_.unknown_0_ & 2))
        return NULL;
    return objects_[idx];
}

GameObject* GameState::GetProtagonist()
{
    return GetGameObjectByIndex(protagonistObjectIndex_);
}

GameObject* GameState::GetUnknownGameObject()
{
    return GetGameObjectByIndex(unknownObjectIndex_397c_);
}

GameObject* GameState::GetPartyMemberByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    if (objects_[idx] == NULL)
        return NULL;
    if (!(objects_[idx]->obj3D_.unknown_0_ & 0x800))
        return NULL;
    return objects_[idx];
}

GameObject* GameState::GetMaybeFieldMonsterByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    if (objects_[idx] == NULL)
        return NULL;
    if (!(objects_[idx]->obj3D_.unknown_0_ & 0x20))
        return NULL;
    return objects_[idx];
}

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