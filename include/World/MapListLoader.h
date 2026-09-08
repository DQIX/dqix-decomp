#pragma once

#include "../Graphics/Vector.h"

// This is different in JPN
struct MapListInfo
{
    char maybeModelName[0xa];
    char buffer2[0x10]; // these aren't populated
    char buffer3[0x10]; 
#if defined(jpn)
    char jp_buffer[0x20];
#endif
    short unknown_2a;
    short unknown_2c;
    char padding_2e[2];
    int unknown_30;
    fix16_t worldRotation; // 0 for every zone except Angel Falls
    char padding_36[2];
    int unknown_38;
    int unknown_3c;
};

void LoadZoneInfoFromMapListScript(int zoneID, MapListInfo* outInfo, const void* script, unsigned int length);