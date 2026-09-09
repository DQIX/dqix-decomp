#include "Combat/Main/BattleList.h"
#include "Resource/GameResources.h"
#include "System/Brightness.h"
#include <globaldefs.h>

// Temporary internal wrappers
extern "C" ARM void *func_020daf90();
extern "C" ARM int func_020db9cc(void *unk, int screen, int brightness, unsigned int duration);
extern "C" ARM void func_020c39a0(volatile unsigned short *reg, int brightness);
extern "C" ARM int func_02010208(BattleStruct *battleStruct);
extern "C" ARM int func_020c39c8(volatile unsigned short *reg);
#define REG_MASTER_BRIGHT ((volatile unsigned short *) 0x0400006C)
#define REG_MASTER_BRIGHT_SUB ((volatile unsigned short *) 0x0400106C)

// helper functions to be inlined, needs to return int to get "? 1 : 0" behaviour
static inline int IsTransitioningMain(GameResources* res) { return res->mainBrightnessTimeRemaining > 0; }
static inline int IsTransitioningSub(GameResources* res) { return res->subBrightnessTimeRemaining > 0; }

// func_0203aee0
extern "C" ARM void InitializeBrightnessState(GameResources* resources) {
    resources->brightnessFlags_0 = 0;
    resources->brightnessFlags_4 = 0;
    resources->brightnessFlags_8 = 0;

    resources->mainBrightnessTimeRemaining = 0;
    resources->subBrightnessTimeRemaining  = 0;

    resources->mainBrightnessLocked = false;
    resources->subBrightnessLocked  = false;
    resources->mainBrightnessDirty  = false;
    resources->subBrightnessDirty   = false;

    resources->allowBrightnessApply = true;

    resources->mainBrightness = (float) func_020c39c8(REG_MASTER_BRIGHT);
    resources->subBrightness = (float) func_020c39c8(REG_MASTER_BRIGHT_SUB);
}

// func_0203af44
extern "C" ARM void Stub() {}

// func_0203af48
extern "C" ARM void UpdateBrightnessTransitions(GameResources* resources) {
    BattleStruct *battleStruct = GetBattleStruct();
    int delta                  = func_02010208(battleStruct);

    if (IsTransitioningMain(resources))
    {
        resources->mainBrightness += (float)delta * (((float)resources->mainBrightnessTarget - resources->mainBrightness) /
                                                    (float)resources->mainBrightnessTimeRemaining);
        resources->mainBrightnessTimeRemaining -= delta;

        if (resources->mainBrightnessTimeRemaining <= 0)
            resources->mainBrightness = (float)resources->mainBrightnessTarget;

        resources->mainBrightnessDirty = true;
    }

    if (IsTransitioningSub(resources))
    {
        resources->subBrightness += (float)delta * (((float)resources->subBrightnessTarget - resources->subBrightness) /
                                                   (float)resources->subBrightnessTimeRemaining);
        resources->subBrightnessTimeRemaining -= delta;

        if (resources->subBrightnessTimeRemaining <= 0)
            resources->subBrightness = (float)resources->subBrightnessTarget;

        resources->subBrightnessDirty = true;
    }
}

// func_0203b080
extern "C" ARM void ApplyBrightness(GameResources* resources) {
    if (resources->allowBrightnessApply == 0) return;

    resources->allowBrightnessApply = 0;

    if (resources->mainBrightnessDirty)
        func_020c39a0(REG_MASTER_BRIGHT, (int) resources->mainBrightness);

    resources->mainBrightnessDirty = false;

    if (resources->subBrightnessDirty)
        func_020c39a0(REG_MASTER_BRIGHT_SUB, (int) resources->subBrightness);

    resources->subBrightnessDirty = false;
}

// func_0203b0f8
extern "C" ARM void UpdateAndApplyBrightness(GameResources* resources) {
    UpdateBrightnessTransitions(resources);
    ApplyBrightness(resources);
}

// func_0203b110
extern "C" ARM void SetMainBrightness(GameResources* resources, int brightness, int duration) {
    if (resources->mainBrightnessLocked)
        return;

    void *unk = func_020daf90();

    if (func_020db9cc(unk, 0, brightness, duration) == 0)
        return;

    if (duration == 0)
    {
        resources->mainBrightness              = (float)brightness;
        resources->mainBrightnessTarget        = brightness;
        resources->mainBrightnessTimeRemaining = 0;
        resources->mainBrightnessDirty         = true;
    }
    else
    {
        resources->mainBrightnessTarget        = brightness;
        resources->mainBrightnessTimeRemaining = (int)((float)duration * 16.667f);
    }
}

// func_0203b19c
extern "C" ARM void SetSubBrightness(GameResources* resources, int brightness, int duration) {
    if (resources->subBrightnessLocked)
        return;

    void *unk = func_020daf90();

    if (func_020db9cc(unk, 1, brightness, duration) == 0)
        return;

    if (duration == 0)
    {
        resources->subBrightness              = (float)brightness;
        resources->subBrightnessTarget        = brightness;
        resources->subBrightnessTimeRemaining = 0;
        resources->subBrightnessDirty         = true;
    }
    else
    {
        resources->subBrightnessTarget        = brightness;
        resources->subBrightnessTimeRemaining = (int)((float)duration * 16.667f);
    }    
}

// func_0203b228
extern "C" ARM void SetBrightness(GameResources* resources, int brightness, int duration) {
    SetMainBrightness(resources, brightness, duration);
    SetSubBrightness(resources, brightness, duration);
}

// func_0203b250
extern "C" ARM void SetAndLockMainBrightness(GameResources* resources, int brightness, int duration) {
    SetMainBrightness(resources, brightness, duration);
    resources->mainBrightnessLocked = true;
}

// func_0203b268
extern "C" ARM void SetAndLockSubBrightness(GameResources* resources, int brightness, int duration) {
    SetSubBrightness(resources, brightness, duration);
    resources->subBrightnessLocked = true;
}

// func_0203b280
extern "C" ARM void SetAndLockBrightness(GameResources* resources, int brightness, int duration) {
    SetMainBrightness(resources, brightness, duration);
    resources->mainBrightnessLocked = true;

    SetSubBrightness(resources, brightness, duration);
    resources->subBrightnessLocked = true;
}

// func_0203b2b8
extern "C" ARM void UnlockAndSetMainBrightness(GameResources* resources, int brightness, int duration) {
    resources->mainBrightnessLocked = false;
    SetMainBrightness(resources, brightness, duration);
}

// func_0203b2cc
extern "C" ARM void UnlockAndSetSubBrightness(GameResources* resources, int brightness, int duration) {
    resources->subBrightnessLocked = false;
    SetSubBrightness(resources, brightness, duration);
}

// func_0203b2e0
extern "C" ARM void UnlockAndSetBrightness(GameResources* resources, int brightness, int duration) {
    resources->mainBrightnessLocked = false;
    SetMainBrightness(resources, brightness, duration);

    resources->subBrightnessLocked = false;
    SetSubBrightness(resources, brightness, duration);
}

// func_0203b318
extern "C" ARM void SetMainBrightnessWithDurationMs(GameResources* resources, int brightness, unsigned int durationMs) {
    if (resources->mainBrightnessLocked)
        return;

    void *unk = func_020daf90();
    unsigned int numFrames = (durationMs * 3) / 100;

    if (func_020db9cc(unk, 0, brightness, numFrames) == 0)
        return;

    if (durationMs != 0)
    {
        resources->mainBrightnessTarget        = brightness;
        resources->mainBrightnessTimeRemaining = durationMs;
    }
    else
    {
        resources->mainBrightness              = (float) brightness;
        resources->mainBrightnessTarget        = brightness;
        resources->mainBrightnessTimeRemaining = 0;
        resources->mainBrightnessDirty         = true;
    }    
}

// func_0203b398
extern "C" ARM int IsMainBrightnessTransitionActive(GameResources* resources) {
    return resources->mainBrightnessTimeRemaining > 0;
}

// func_0203b3ac
extern "C" ARM int IsSubBrightnessTransitionActive(GameResources* resources) {
    return resources->subBrightnessTimeRemaining > 0;
}

// func_0203b3c0
extern "C" ARM int IsBrightnessTransitionActive(GameResources* resources) {
    return IsTransitioningMain(resources) || IsTransitioningSub(resources);
}

// func_0203b400
extern "C" ARM int GetMainBrightnessTransitionState(GameResources* resources) {
    if (!IsTransitioningMain(resources))
        return 0;

    int state = 1;

    if (resources->mainBrightnessTarget < 0)
        state = 2;
    if (resources->mainBrightnessTarget > 0)
        state = 3;

    return state;
}

// func_0203b438
extern "C" ARM int GetSubBrightnessTransitionState(GameResources* resources) {
    if (!IsTransitioningSub(resources))
        return 0;

    int state = 1;

    if (resources->subBrightnessTarget < 0)
        state = 2;
    if (resources->subBrightnessTarget > 0)
        state = 3;

    return state;
}

// func_0203b470
extern "C" ARM unsigned short GetBrightnessTransitionStates(GameResources* resources) {
    int mainState = GetMainBrightnessTransitionState(resources);
    int subState  = GetSubBrightnessTransitionState(resources);

    return mainState | (subState << 8);
}