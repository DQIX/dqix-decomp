#pragma once

#include "../Memory/SafeAllocator.h"
#include "Vector.h"
#include "std_library_functions.h"
#include "LightingManager.h"

// I have no idea what any of this is. I thought it would be more fruitful
// but it's pretty much a dead end, I don't even see where this data gets 
// consumed. Disabling it causes weird lighting effects, that's all I know really

// sizeof == 0x30c
class LightingInfo
{
public:
    union
    {
        LightingManager::BasicLighting basic_;
        LightingManager::AdvancedLighting advanced_;
    };

    LightingManager::FogList fogList;
    int maybeMode_; // 1 = use small struct, 2 = use big
    float gradientCenterOffset_;

    // Initialize() doesn't return *this, so it isn't a constructor, but
    // it's called by Zone3D constructor, so our constructor must be inline and call it
    LightingInfo() { Initialize(); }

    void Initialize();
    void Reset();
    void LoadFromScript(const void* script, unsigned int length, SafeAllocator* alloc);
};