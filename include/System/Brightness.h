#pragma once

#include "Combat/Main/UnknownContext.h"
#include <globaldefs.h>

extern "C" ARM void SetMainBrightness(UnknownContext *context, int brightness, int duration);

extern "C" ARM void SetSubBrightness(UnknownContext *context, int brightness, int duration);

extern "C" ARM void SetBrightness(UnknownContext *context, int brightness, int duration);

extern "C" ARM void SetAndLockMainBrightness(UnknownContext *context, int brightness, int duration);

extern "C" ARM void SetAndLockSubBrightness(UnknownContext *context, int brightness, int duration);

extern "C" ARM void SetAndLockBrightness(UnknownContext *context, int brightness, int duration);

extern "C" ARM void UnlockAndSetMainBrightness(UnknownContext *context, int brightness, int duration);

extern "C" ARM void UnlockAndSetSubBrightness(UnknownContext *context, int brightness, int duration);

extern "C" ARM void UnlockAndSetBrightness(UnknownContext *context, int brightness, int duration);

extern "C" ARM void SetMainBrightnessWithDurationMs(UnknownContext *context, int brightness, unsigned int durationMs);

extern "C" ARM int IsMainBrightnessTransitionActive(UnknownContext *context);
extern "C" ARM int IsSubBrightnessTransitionActive(UnknownContext *context);
extern "C" ARM int IsBrightnessTransitionActive(UnknownContext *context);

extern "C" ARM int GetMainBrightnessTransitionState(UnknownContext *context);
extern "C" ARM int GetSubBrightnessTransitionState(UnknownContext *context);

extern "C" ARM unsigned short GetBrightnessTransitionStates(UnknownContext *context);