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

// func_0203aee0
extern "C" ARM void InitializeBrightnessState(GameResources *resources) {
    resources->flags_00 = 0;
    resources->flags_04 = 0;
    resources->flags_08 = 0;

    resources->mainBrightnessTimeRemaining = 0;
    resources->subBrightnessTimeRemaining  = 0;

    resources->mainBrightnessLocked = 0;
    resources->subBrightnessLocked  = 0;
    resources->mainBrightnessDirty  = 0;
    resources->subBrightnessDirty   = 0;

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

    int active;

    if (resources->mainBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active != 0) {
        resources->mainBrightness += (float) delta * (((float) resources->mainBrightnessTarget - resources->mainBrightness) /
                                                    (float) resources->mainBrightnessTimeRemaining);

        resources->mainBrightnessTimeRemaining -= delta;

        if (resources->mainBrightnessTimeRemaining <= 0) resources->mainBrightness = (float) resources->mainBrightnessTarget;

        resources->mainBrightnessDirty = 1;
    }

    if (resources->subBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active == 0) return;

    resources->subBrightness += (float) delta * (((float) resources->subBrightnessTarget - resources->subBrightness) /
                                               (float) resources->subBrightnessTimeRemaining);

    resources->subBrightnessTimeRemaining -= delta;

    if (resources->subBrightnessTimeRemaining <= 0) resources->subBrightness = (float) resources->subBrightnessTarget;

    resources->subBrightnessDirty = 1;
}

// func_0203b080
extern "C" ARM void ApplyBrightness(GameResources* resources) {
    if (resources->allowBrightnessApply == 0) return;

    resources->allowBrightnessApply = 0;

    if (resources->mainBrightnessDirty != 0) {
        func_020c39a0(REG_MASTER_BRIGHT, (int) resources->mainBrightness);
    }

    resources->mainBrightnessDirty = 0;

    if (resources->subBrightnessDirty != 0) {
        func_020c39a0(REG_MASTER_BRIGHT_SUB, (int) resources->subBrightness);
    }

    resources->subBrightnessDirty = 0;
}

// func_0203b0f8
extern "C" ARM void UpdateAndApplyBrightness(GameResources* resources) {
    UpdateBrightnessTransitions(resources);
    ApplyBrightness(resources);
}

// func_0203b110
extern "C" ARM void SetMainBrightness(GameResources* resources, int brightness, int duration) {
    if (resources->mainBrightnessLocked != 0) return;

    void *unk = func_020daf90();

    if (func_020db9cc(unk, 0, brightness, duration) == 0) return;

    if (duration == 0) {
        resources->mainBrightness              = (float) brightness;
        resources->mainBrightnessTarget        = brightness;
        resources->mainBrightnessTimeRemaining = 0;
        resources->mainBrightnessDirty         = 1;
        return;
    }

    resources->mainBrightnessTarget        = brightness;
    resources->mainBrightnessTimeRemaining = (int) ((float) duration * 16.667f);
}

// func_0203b19c
extern "C" ARM void SetSubBrightness(GameResources* resources, int brightness, int duration) {
    if (resources->subBrightnessLocked != 0) return;

    void *unk = func_020daf90();

    if (func_020db9cc(unk, 1, brightness, duration) == 0) return;

    if (duration == 0) {
        resources->subBrightness              = (float) brightness;
        resources->subBrightnessTarget        = brightness;
        resources->subBrightnessTimeRemaining = 0;
        resources->subBrightnessDirty         = 1;
        return;
    }

    resources->subBrightnessTarget        = brightness;
    resources->subBrightnessTimeRemaining = (int) ((float) duration * 16.667f);
}

// func_0203b228
extern "C" ARM void SetBrightness(GameResources* resources, int brightness, int duration) {
    SetMainBrightness(resources, brightness, duration);
    SetSubBrightness(resources, brightness, duration);
}

// func_0203b250
extern "C" ARM void SetAndLockMainBrightness(GameResources* resources, int brightness, int duration) {
    SetMainBrightness(resources, brightness, duration);
    resources->mainBrightnessLocked = 1;
}

// func_0203b268
extern "C" ARM void SetAndLockSubBrightness(GameResources* resources, int brightness, int duration) {
    SetSubBrightness(resources, brightness, duration);
    resources->subBrightnessLocked = 1;
}

// func_0203b280
extern "C" ARM void SetAndLockBrightness(GameResources* resources, int brightness, int duration) {
    SetMainBrightness(resources, brightness, duration);
    resources->mainBrightnessLocked = 1;

    SetSubBrightness(resources, brightness, duration);
    resources->subBrightnessLocked = 1;
}

// func_0203b2b8
extern "C" ARM void UnlockAndSetMainBrightness(GameResources* resources, int brightness, int duration) {
    resources->mainBrightnessLocked = 0;
    SetMainBrightness(resources, brightness, duration);
}

// func_0203b2cc
extern "C" ARM void UnlockAndSetSubBrightness(GameResources* resources, int brightness, int duration) {
    resources->subBrightnessLocked = 0;
    SetSubBrightness(resources, brightness, duration);
}

// func_0203b2e0
extern "C" ARM void UnlockAndSetBrightness(GameResources* resources, int brightness, int duration) {
    resources->mainBrightnessLocked = 0;
    SetMainBrightness(resources, brightness, duration);

    resources->subBrightnessLocked = 0;
    SetSubBrightness(resources, brightness, duration);
}

// func_0203b318
extern "C" ARM void SetMainBrightnessWithDurationMs(GameResources* resources, int brightness, unsigned int durationMs) {
    if (resources->mainBrightnessLocked != 0) return;

    void *unk = func_020daf90();

    unsigned int scaledDuration = (durationMs * 3) / 100;

    if (func_020db9cc(unk, 0, brightness, scaledDuration) == 0) return;

    if (durationMs != 0) {
        resources->mainBrightnessTarget        = brightness;
        resources->mainBrightnessTimeRemaining = durationMs;
        return;
    }

    resources->mainBrightness              = (float) brightness;
    resources->mainBrightnessTarget        = brightness;
    resources->mainBrightnessTimeRemaining = 0;
    resources->mainBrightnessDirty         = 1;
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
    int active;

    if (resources->mainBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active != 0) goto active_transition;

    if (resources->subBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active == 0) goto no_transition;

active_transition:
    return 1;

no_transition:
    return 0;
}

// func_0203b400
extern "C" ARM int GetMainBrightnessTransitionState(GameResources* resources) {
    int active;

    if (resources->mainBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active == 0) return 0;

    int state = 1;

    if (resources->mainBrightnessTarget < 0) state = 2;

    if (resources->mainBrightnessTarget > 0) state = 3;

    return state;
}

// func_0203b438
extern "C" ARM int GetSubBrightnessTransitionState(GameResources* resources) {
    int active;

    if (resources->subBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active == 0) return 0;

    int state = 1;

    if (resources->subBrightnessTarget < 0) state = 2;

    if (resources->subBrightnessTarget > 0) state = 3;

    return state;
}

// func_0203b470
extern "C" ARM unsigned short GetBrightnessTransitionStates(GameResources* resources) {
    int mainState = GetMainBrightnessTransitionState(resources);
    int subState  = GetSubBrightnessTransitionState(resources);

    return mainState | (subState << 8);
}