#pragma once

#include "Vector.h"
#include "std_library_functions.h"
#include "../World/Object3D.h"

class AtmosphericEffect
{
public:
    Object3D object_;
    char name_[0x20];
    fix32_t scaleFactor_; // cc
    fix32_t currentXOffset_; // d0
    fix32_t currentZOffset_; // d4
    fix32_t xSpeed_; // d8
    fix32_t zSpeed_; // dc
    fix32_t originX_; // e0
    fix32_t originZ_; // e4
    fix32_t tileWidth_; // e8
    fix32_t tileHeight_; // ec
    // bit 0: scrolls in negative y-direction (up)
    // bit 1: scrolls in y-direction (not needed if bit 0 set)
    // bit 2: scrolls in negative x-direction (left)
    // bit 3: scrolls in x-direction (not needed if bit 2 set)
    unsigned short flags_;
    unsigned char tileCountX_;
    unsigned char tileCountZ_;
    unsigned int archiveFileSize_; // used both for cmed and chr
    const void* archiveFileData_; // used both for cmed and chr
    unsigned char alpha_;
    unsigned char padding_fd[3];
    AtmosphericEffect* pNext_;

    void Initialize();
};

// sizeof == 0x18.
// Used for rendering front-of-screen effects, notably the rain on Pluvi Isle,
// the snow in Snowberia/Cringle Coast, but also the sepia effect during
// flashback cutscenes.
class AtmosphericEffectSet
{
public:
    AtmosphericEffect* firstThing_; 
    int archiveLoadHandle_;
    void* rawArchive_;
    unsigned int archiveLength_;
    unsigned char unknown_10_;
    // bit 0: disable entirely
    // bit 1: fading in?
    // bit 2: fading out?
    unsigned char effectFlags_;
    unsigned char fadeTimeRemaining_;
    // Each object has its own alpha, but this is an overall alpha and the
    // two get multiplied
    unsigned char alpha_;
    unsigned char timeOfDayIndex_ : 2;
    unsigned char unknown_14_bit_2_ : 1;
    // if bit k of this is set, then the effect will be hidden when the time of
    // day is equal to k (0 = night, 1 = morning, 2 = day, 3 = evening)
    unsigned char timeOfDayHideBits_ : 4;
    // if set, the visibility will be updated to match the time of day and will
    // change in a single frame instead of fading
    unsigned char visibilityDirty_ : 1;

    char padding_15[3];

    void Reset();

    // name is e.g. "F17E1" (on Pluvi Isle)
    void LoadArchive(const char* name);
    bool IsArchiveLoaded();

    void ProcessArchive(AtmosphericEffectSet* alt, SafeAllocator* allocator, void* pZone50Thing);
    bool Draw();

    void AddEffectObject(AtmosphericEffect* obj);
    AtmosphericEffect* GetEffectByName(const char* name);
    AtmosphericEffect* GetFirstEffect();

    // Fading happens at a constant rate of 31 alpha levels per 90 ticks.
    // So e.g. if alpha = 10 initially it'll take ~60 ticks to fade in and
    // ~30 ticks to fade out
    void AdvanceAlphaFade();
    void StartFadeOut();
    void StartFadeIn();

    // These both use some external data and don't really make sense atm
    void DetermineVisibilityFromTimeOfDay();
    void DetermineVisibilityFromUnknown();
};