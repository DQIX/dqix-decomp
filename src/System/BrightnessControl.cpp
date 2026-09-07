#include "Combat/Main/UnknownContext.h"
#include "System/Brightness.h"
#include "System/BrightnessWrappers.h"
#include <globaldefs.h>

extern "C" ARM void *func_020daf90();
extern "C" ARM int func_020db9cc(void *unk, int screen, int brightness, unsigned int duration);


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
    context->mainBrightnessTimeRemaining = (int) ((float) duration * 16.66699981689453125f);
}

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
    context->subBrightnessTimeRemaining = (int) ((float) duration * 16.66699981689453125f);
}

extern "C" ARM void SetBrightness(UnknownContext *context, int brightness, int duration) {
    SetMainBrightness(context, brightness, duration);
    SetSubBrightness(context, brightness, duration);
}

extern "C" ARM void SetAndLockMainBrightness(UnknownContext *context, int brightness, int duration) {
    SetMainBrightness(context, brightness, duration);
    context->mainBrightnessLocked = 1;
}

extern "C" ARM void SetAndLockSubBrightness(UnknownContext *context, int brightness, int duration) {
    SetSubBrightness(context, brightness, duration);
    context->subBrightnessLocked = 1;
}

extern "C" ARM void SetAndLockBrightness(UnknownContext *context, int brightness, int duration) {
    SetMainBrightness(context, brightness, duration);
    context->mainBrightnessLocked = 1;

    SetSubBrightness(context, brightness, duration);
    context->subBrightnessLocked = 1;
}

extern "C" ARM void UnlockAndSetMainBrightness(UnknownContext *context, int brightness, int duration) {
    context->mainBrightnessLocked = 0;
    SetMainBrightness(context, brightness, duration);
}

extern "C" ARM void UnlockAndSetSubBrightness(UnknownContext *context, int brightness, int duration) {
    context->subBrightnessLocked = 0;
    SetSubBrightness(context, brightness, duration);
}

extern "C" ARM void UnlockAndSetBrightness(UnknownContext *context, int brightness, int duration) {
    context->mainBrightnessLocked = 0;
    SetMainBrightness(context, brightness, duration);

    context->subBrightnessLocked = 0;
    SetSubBrightness(context, brightness, duration);
}

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

extern "C" ARM int IsMainBrightnessTransitionActive(UnknownContext* context)
{
    return context->mainBrightnessTimeRemaining > 0;
}

extern "C" ARM int IsSubBrightnessTransitionActive(UnknownContext* context)
{
    return context->subBrightnessTimeRemaining > 0;
}

extern "C" ARM int IsBrightnessTransitionActive(UnknownContext* context)
{
    int active;

    if (context->mainBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active != 0)
        goto active_transition;

    if (context->subBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active == 0)
        goto no_transition;

active_transition:
    return 1;

no_transition:
    return 0;
}

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

extern "C" ARM unsigned short GetBrightnessTransitionStates(UnknownContext* context)
{
    int mainState = GetMainBrightnessTransitionState(context);
    int subState = GetSubBrightnessTransitionState(context);

    return mainState | (subState << 8);
}