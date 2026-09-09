#pragma once

#include "Resource/GameResources.h"
#include <globaldefs.h>

void InitializeBrightnessState(GameResources* resources);

void UpdateBrightnessTransitions(GameResources* resources);

void ApplyBrightness(GameResources* resources);

void UpdateAndApplyBrightness(GameResources* resources);

void SetMainBrightness(GameResources* resources, int brightness, int duration);

void SetSubBrightness(GameResources* resources, int brightness, int duration);

void SetBrightness(GameResources* resources, int brightness, int duration);

void SetAndLockMainBrightness(GameResources* resources, int brightness, int duration);

void SetAndLockSubBrightness(GameResources* resources, int brightness, int duration);

void SetAndLockBrightness(GameResources* resources, int brightness, int duration);

void UnlockAndSetMainBrightness(GameResources* resources, int brightness, int duration);

void UnlockAndSetSubBrightness(GameResources* resources, int brightness, int duration);

void UnlockAndSetBrightness(GameResources* resources, int brightness, int duration);

void SetMainBrightnessWithDurationMs(GameResources* resources, int brightness, unsigned int durationMs);

// might be returning bool instead of int
int IsMainBrightnessTransitionActive(GameResources* resources);
int IsSubBrightnessTransitionActive(GameResources* resources);
int IsBrightnessTransitionActive(GameResources* resources);

int GetMainBrightnessTransitionState(GameResources* resources);
int GetSubBrightnessTransitionState(GameResources* resources);

unsigned short GetBrightnessTransitionStates(GameResources* resources);