#pragma once

#include "Resource/GameResources.h"
#include <globaldefs.h>

extern "C" ARM void InitializeBrightnessState(GameResources* resources);

extern "C" ARM void UpdateBrightnessTransitions(GameResources* resources);

extern "C" ARM void ApplyBrightness(GameResources* resources);

extern "C" ARM void UpdateAndApplyBrightness(GameResources* resources);

extern "C" ARM void SetMainBrightness(GameResources* resources, int brightness, int duration);

extern "C" ARM void SetSubBrightness(GameResources* resources, int brightness, int duration);

extern "C" ARM void SetBrightness(GameResources* resources, int brightness, int duration);

extern "C" ARM void SetAndLockMainBrightness(GameResources* resources, int brightness, int duration);

extern "C" ARM void SetAndLockSubBrightness(GameResources* resources, int brightness, int duration);

extern "C" ARM void SetAndLockBrightness(GameResources* resources, int brightness, int duration);

extern "C" ARM void UnlockAndSetMainBrightness(GameResources* resources, int brightness, int duration);

extern "C" ARM void UnlockAndSetSubBrightness(GameResources* resources, int brightness, int duration);

extern "C" ARM void UnlockAndSetBrightness(GameResources* resources, int brightness, int duration);

extern "C" ARM void SetMainBrightnessWithDurationMs(GameResources* resources, int brightness, unsigned int durationMs);

extern "C" ARM int IsMainBrightnessTransitionActive(GameResources* resources);
extern "C" ARM int IsSubBrightnessTransitionActive(GameResources* resources);
extern "C" ARM int IsBrightnessTransitionActive(GameResources* resources);

extern "C" ARM int GetMainBrightnessTransitionState(GameResources* resources);
extern "C" ARM int GetSubBrightnessTransitionState(GameResources* resources);

extern "C" ARM unsigned short GetBrightnessTransitionStates(GameResources* resources);