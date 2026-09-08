#pragma once

struct UnknownContext {
    // full size: 0x44C8 bytes

    unsigned int flags_00;
    unsigned int flags_04;
    unsigned int flags_08;

    float mainBrightness;
    int mainBrightnessTarget;
    int mainBrightnessTimeRemaining;

    float subBrightness;
    int subBrightnessTarget;
    int subBrightnessTimeRemaining;

    unsigned char mainBrightnessLocked;
    unsigned char subBrightnessLocked;
    unsigned char mainBrightnessDirty;
    unsigned char subBrightnessDirty;
    unsigned char allowBrightnessApply;

     // ... ~0x449F bytes still to understand
};