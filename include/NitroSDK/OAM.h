#pragma once

#include <std_library_functions.h>

// https://problemkaputt.de/gbatek.htm#lcdobjoamattributes describes gba oam
// slight differences arise for nds case, see:
// https://problemkaputt.de/gbatek.htm#dsvideoobjs
struct OAMEntry
{
    // bits 0-7: y coordinate
    // bit 8: enable rotation/scaling
    // bit 9: (if rot/scale enabled: ) double size, (if rot/scale disabled: ) hide
    // bits 10-11: mode (normal / semi-transparent / obj window / bitmap)
    // bit 12: mosaic
    // bit 13: use 256-color instead of 16x 16-color palette
    // bits 14-15: shape (square / horizontal / vertical)
    uint16_t word0;
    // bits 0-8: x-coordinate
    // if rot/scale enabled:
    //     bits 9-13: rot/scale parameter selection index
    // else:
    //     bit 12: flip horizontal
    //     bit 13: flip vertical
    // bits 14-15: size. depends on shape.
    //     square: 8x8 / 16x16 / 32x32 / 64x64
    //     horizontal: 16x8 / 32x8 / 32x16 / 64x32
    //     vertical: 8x16 / 8x32 / 16x32 / 32x64
    uint16_t word1;
    // bits 0-9: character name/index of first tile?
    // bits 10-11: priority relative to BG
    // bits 12-15: palette index in 16-color mode
    uint16_t word2;
};

union OAMEntryEXT
{
    struct
    {
        OAMEntry base;
        uint16_t rotScaleData;
    };
    uint32_t words01;
};