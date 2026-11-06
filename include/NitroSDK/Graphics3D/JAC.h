#pragma once

#include "Common.h"
#include "Animation.h"

struct Nitro3DJAC
{
    Nitro3DAnimationSignature signature; // "J\0AC"
    uint16_t numFrames;
    uint16_t numTracks;
    // bit 0: interpolate smoothly?
    // bit 1: loop animation
    uint32_t flags;
    uint32_t pivotDataOffset;
    uint32_t basisMatricesOffset;

    uint16_t trackOffsets[1];

    struct Track
    {
        // bit 0: no channels at all
        // bits 1,2: no translation channels
        // bit 3: x-translation is constant
        // bit 4: y-translation is constant
        // bit 5: z-translation is constant
        // bits 6,7: no rotation channel
        // bit 8: rotation is constant
        // bits 9,10: no scale channels
        // bit 11: x-scale is constant
        // bit 12: y-scale is constant
        // bit 13: z-scale is constant
        // bits 14-23: ???, unused?
        // bits 24-31: index of bone matrix to target
        uint32_t flagsAndTargetBoneMatrix;

        // channel data goes here...

        struct ChannelNonConst
        {
            // bits 0-15: start frame
            // bits 16-27: end frame
            // bits 28-29: 'width', determines type of data
            // bits 30-31: log(rate)
            uint32_t metadata;
            uint32_t samplesOffset;
        };
    };

    struct ScaleSample16
    {
        fix16_t primary;
        fix16_t secondary;
    };

    struct ScaleSample32
    {
        fix32_t primary;
        fix32_t secondary;
    };

    struct PivotMatrix
    {
        // bits 0-3: form (values 0 to 8)
        // bit 4: unit is -1
        // bit 5: c = -b instead of c = b
        // bit 6: d = -a instead of d = a
        int16_t flags;
        fix16_t a;
        fix16_t b;
    };

    struct BasisMatrix
    {
        int16_t data[5];
    };

    inline PivotMatrix* GetPivotMatrices() const
    {
        return (PivotMatrix*)((intptr_t)this + this->pivotDataOffset);
    }

    inline BasisMatrix* GetBasisMatrices() const
    {
        return (BasisMatrix*)((intptr_t)this + this->basisMatricesOffset);
    }
};