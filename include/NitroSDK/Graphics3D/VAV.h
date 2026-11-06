#pragma once

#include "Common.h"
#include "Animation.h"

// not well documented at all, and doesn't seem to be used in DQIX anywhere.
// signature is V.AV and controls visibility by interacting with render command 2.
struct Nitro3DVAV
{
    char unk_0[6];
    unsigned short numConditions;
    char unk_8[4];
    // to determine visibility, you generate a test index via
    // testIndex = (frame * numConditions) + conditionIndex
    // then look up bitfield[testIndex >> 5] & (1 << (testIndex & 0x1f)).
    // if it's nonzero, the model is visible
    uint32_t bitfield[1];
};