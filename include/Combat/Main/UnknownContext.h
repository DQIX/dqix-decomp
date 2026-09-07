#pragma once

struct UnknownContext
{
    unsigned int flags_00;
    unsigned int flags_04;
    unsigned int flags_08;

    float mainBrightness;
    int mainBrightnessTarget;
    int mainBrightnessTimeRemaining;

    float subBrightness;
    int subBrightnessTarget;
    int subBrightnessTimeRemaining;

    unsigned char unk24;
    unsigned char unk25;
    unsigned char mainBrightnessDirty;
    unsigned char subBrightnessDirty;
    unsigned char unk28;
};