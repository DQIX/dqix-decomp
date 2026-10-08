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

unsigned int GetFlags00(GameResources* resources);
void SetFlags00(GameResources* resources, unsigned int mask);
void ClearFlags00(GameResources* resources, unsigned int mask);

// Known masks:
// 0x2 = Uncertain. Set during normal free roam; restored after transitions and battles.
// 0x4 = Battle runtime state. Set during initialization, toggled during setup, cleared on teardown.
// 0x10 = Temporary flag used during scene transitions and battle state changes.
// 0x20 = Uncertain. Set during battle setup and cleared on teardown.
// 0x800 = Combat entry/handoff. Set early on entry and cleared before battle is established.
unsigned int TestFlags00(GameResources* resources, unsigned int mask);

unsigned int GetFlags04(GameResources* resources);
void SetFlags04(GameResources* resources, unsigned int mask);
void ClearFlags04(GameResources* resources, unsigned int mask);

// Known masks:
// 0x1 = Scene/zone transition. Set on entry and cleared when field control resumes.
// 0x2 = Battle runtime state. Set during setup and cleared on teardown.
// 0x4 = Battle initialization. Temporarily suppresses some Object3D/world processing.
// 0x8 = Unknown. Seen in larger initialization masks.
// 0x10 = Uncertain battle/modal state. Set during battle setup and cleared on teardown.
// 0x20 = Unknown. Seen in larger initialization masks.
// 0x40 = Skips field Object3D positioning/rendering in func_ov017_0219ad84 during menus and battle.
// 0x80 = Suppresses some field/render/update processing during menus, battle, and chest interactions.
// 0xC0 = 0x40 | 0x80. Set while the field menu is active and cleared on close.
// 0x100 = Unknown. Seen in larger initialization masks.
// 0x400 = Unknown. Cleared around battle actions; not observed being set in the tested battle.
// 0x800 = Inverse of map flags_0E bit 7. Cleared in battle and restored on return to the field.
// 0x20000 = Unknown. Seen in larger initialization masks.
// 0x40000 = Unknown. Seen in larger initialization masks.
// 0x200000 = Unknown. Cleared during battle exit in the tested flow.
// 0x609FE = Bulk initialization/runtime mask for flags_04.
unsigned int TestFlags04(GameResources* resources, unsigned int mask);

unsigned int GetFlags08(GameResources* resources);
void SetFlags08(GameResources* resources, unsigned int mask);
void ClearFlags08(GameResources* resources, unsigned int mask);

// Known masks:
// 0x2 = Uncertain. Relaxes some field_06 object/candidate matching requirements outside battle.
// 0x4 = BATTLE_ACTIVE. Set when battle becomes active and cleared when combat ends.
// 0x8 = Uncertain pending/state-machine flag. Clearing is partly linked to brightness transitions.
unsigned int TestFlags08(GameResources* resources, unsigned int mask);

void WriteBrightnessToHardware(GameResources* gameResources);
int GetBrightness(GameResources* gameResources, bool useSubScreen);
bool IsBrightnessWithinValidRange(GameResources* gameResources, bool useSubScreen);
bool IsBrightnessOutsideValidRange(GameResources* gameResources, bool useSubScreen);
bool IsBrightnessZero(GameResources* gameResources, bool useSubScreen);
