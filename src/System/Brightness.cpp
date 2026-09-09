#include "Combat/Main/BattleList.h"
#include "Combat/Main/UnknownContext.h"
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
extern "C" ARM void InitializeBrightnessState(UnknownContext *context) {
    context->flags_00 = 0;
    context->flags_04 = 0;
    context->flags_08 = 0;

    context->mainBrightnessTimeRemaining = 0;
    context->subBrightnessTimeRemaining  = 0;

    context->mainBrightnessLocked = 0;
    context->subBrightnessLocked  = 0;
    context->mainBrightnessDirty  = 0;
    context->subBrightnessDirty   = 0;

    context->allowBrightnessApply = 1;

    context->mainBrightness = (float) func_020c39c8(REG_MASTER_BRIGHT);

    context->subBrightness = (float) func_020c39c8(REG_MASTER_BRIGHT_SUB);
}

// func_0203af44
extern "C" ARM void Stub() {}

// func_0203af48
extern "C" ARM void UpdateBrightnessTransitions(UnknownContext *context) {
    BattleStruct *battleStruct = GetBattleStruct();
    int delta                  = func_02010208(battleStruct);

    int active;

    if (context->mainBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active != 0) {
        context->mainBrightness += (float) delta * (((float) context->mainBrightnessTarget - context->mainBrightness) /
                                                    (float) context->mainBrightnessTimeRemaining);

        context->mainBrightnessTimeRemaining -= delta;

        if (context->mainBrightnessTimeRemaining <= 0) context->mainBrightness = (float) context->mainBrightnessTarget;

        context->mainBrightnessDirty = 1;
    }

    if (context->subBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active == 0) return;

    context->subBrightness += (float) delta * (((float) context->subBrightnessTarget - context->subBrightness) /
                                               (float) context->subBrightnessTimeRemaining);

    context->subBrightnessTimeRemaining -= delta;

    if (context->subBrightnessTimeRemaining <= 0) context->subBrightness = (float) context->subBrightnessTarget;

    context->subBrightnessDirty = 1;
}

// func_0203b080
extern "C" ARM void ApplyBrightness(UnknownContext *context) {
    if (context->allowBrightnessApply == 0) return;

    context->allowBrightnessApply = 0;

    if (context->mainBrightnessDirty != 0) {
        func_020c39a0(REG_MASTER_BRIGHT, (int) context->mainBrightness);
    }

    context->mainBrightnessDirty = 0;

    if (context->subBrightnessDirty != 0) {
        func_020c39a0(REG_MASTER_BRIGHT_SUB, (int) context->subBrightness);
    }

    context->subBrightnessDirty = 0;
}

// func_0203b0f8
extern "C" ARM void UpdateAndApplyBrightness(UnknownContext *context) {
    UpdateBrightnessTransitions(context);
    ApplyBrightness(context);
}

// func_0203b110
extern "C" ARM void SetMainBrightness(UnknownContext *context, int brightness, int duration) {
    if (context->mainBrightnessLocked != 0) return;

    void *unk = func_020daf90();

    if (func_020db9cc(unk, 0, brightness, duration) == 0) return;

    if (duration == 0) {
        context->mainBrightness              = (float) brightness;
        context->mainBrightnessTarget        = brightness;
        context->mainBrightnessTimeRemaining = 0;
        context->mainBrightnessDirty         = 1;
        return;
    }

    context->mainBrightnessTarget        = brightness;
    context->mainBrightnessTimeRemaining = (int) ((float) duration * 16.667f);
}

// func_0203b19c
extern "C" ARM void SetSubBrightness(UnknownContext *context, int brightness, int duration) {
    if (context->subBrightnessLocked != 0) return;

    void *unk = func_020daf90();

    if (func_020db9cc(unk, 1, brightness, duration) == 0) return;

    if (duration == 0) {
        context->subBrightness              = (float) brightness;
        context->subBrightnessTarget        = brightness;
        context->subBrightnessTimeRemaining = 0;
        context->subBrightnessDirty         = 1;
        return;
    }

    context->subBrightnessTarget        = brightness;
    context->subBrightnessTimeRemaining = (int) ((float) duration * 16.667f);
}

// func_0203b228
extern "C" ARM void SetBrightness(UnknownContext *context, int brightness, int duration) {
    SetMainBrightness(context, brightness, duration);
    SetSubBrightness(context, brightness, duration);
}

// func_0203b250
extern "C" ARM void SetAndLockMainBrightness(UnknownContext *context, int brightness, int duration) {
    SetMainBrightness(context, brightness, duration);
    context->mainBrightnessLocked = 1;
}

// func_0203b268
extern "C" ARM void SetAndLockSubBrightness(UnknownContext *context, int brightness, int duration) {
    SetSubBrightness(context, brightness, duration);
    context->subBrightnessLocked = 1;
}

// func_0203b280
extern "C" ARM void SetAndLockBrightness(UnknownContext *context, int brightness, int duration) {
    SetMainBrightness(context, brightness, duration);
    context->mainBrightnessLocked = 1;

    SetSubBrightness(context, brightness, duration);
    context->subBrightnessLocked = 1;
}

// func_0203b2b8
extern "C" ARM void UnlockAndSetMainBrightness(UnknownContext *context, int brightness, int duration) {
    context->mainBrightnessLocked = 0;
    SetMainBrightness(context, brightness, duration);
}

// func_0203b2cc
extern "C" ARM void UnlockAndSetSubBrightness(UnknownContext *context, int brightness, int duration) {
    context->subBrightnessLocked = 0;
    SetSubBrightness(context, brightness, duration);
}

// func_0203b2e0
extern "C" ARM void UnlockAndSetBrightness(UnknownContext *context, int brightness, int duration) {
    context->mainBrightnessLocked = 0;
    SetMainBrightness(context, brightness, duration);

    context->subBrightnessLocked = 0;
    SetSubBrightness(context, brightness, duration);
}

// func_0203b318
extern "C" ARM void SetMainBrightnessWithDurationMs(UnknownContext *context, int brightness, unsigned int durationMs) {
    if (context->mainBrightnessLocked != 0) return;

    void *unk = func_020daf90();

    unsigned int scaledDuration = (durationMs * 3) / 100;

    if (func_020db9cc(unk, 0, brightness, scaledDuration) == 0) return;

    if (durationMs != 0) {
        context->mainBrightnessTarget        = brightness;
        context->mainBrightnessTimeRemaining = durationMs;
        return;
    }

    context->mainBrightness              = (float) brightness;
    context->mainBrightnessTarget        = brightness;
    context->mainBrightnessTimeRemaining = 0;
    context->mainBrightnessDirty         = 1;
}

// func_0203b398
extern "C" ARM int IsMainBrightnessTransitionActive(UnknownContext *context) {
    return context->mainBrightnessTimeRemaining > 0;
}

// func_0203b3ac
extern "C" ARM int IsSubBrightnessTransitionActive(UnknownContext *context) {
    return context->subBrightnessTimeRemaining > 0;
}

// func_0203b3c0
extern "C" ARM int IsBrightnessTransitionActive(UnknownContext *context) {
    int active;

    if (context->mainBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active != 0) goto active_transition;

    if (context->subBrightnessTimeRemaining > 0)
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
extern "C" ARM int GetMainBrightnessTransitionState(UnknownContext *context) {
    int active;

    if (context->mainBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active == 0) return 0;

    int state = 1;

    if (context->mainBrightnessTarget < 0) state = 2;

    if (context->mainBrightnessTarget > 0) state = 3;

    return state;
}

// func_0203b438
extern "C" ARM int GetSubBrightnessTransitionState(UnknownContext *context) {
    int active;

    if (context->subBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active == 0) return 0;

    int state = 1;

    if (context->subBrightnessTarget < 0) state = 2;

    if (context->subBrightnessTarget > 0) state = 3;

    return state;
}

// func_0203b470
extern "C" ARM unsigned short GetBrightnessTransitionStates(UnknownContext *context) {
    int mainState = GetMainBrightnessTransitionState(context);
    int subState  = GetSubBrightnessTransitionState(context);

    return mainState | (subState << 8);
}