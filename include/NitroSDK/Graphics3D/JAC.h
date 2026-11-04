#pragma once

#include "Common.h"
#include "Animation.h"

struct Nitro3DJAC
{
    Nitro3DAnimationSignature signature_; // "J\0AC"
    uint16_t numFrames_;
    uint16_t numTracks_;
    uint32_t unk_8;
    uint32_t pivotDataOffset_;
    uint32_t basisMatricesOffset_;

    uint16_t trackOffsets_[1];

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
        uint32_t flagsAndTargetBoneMatrix_;

        // channel data goes here...

        struct ChannelNonConst
        {
            // bits 0-15: start frame
            // bits 16-27: end frame
            // bits 28-29: 'width', determines type of data
            // bits 30-31: log(rate)
            uint32_t metadata_;
            uint32_t samplesOffset_;
        };
    };

    struct ScaleSample16
    {
        fix16_t primary_;
        fix16_t secondary_;
    };

    struct ScaleSample32
    {
        fix32_t primary_;
        fix32_t secondary_;
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
        return (PivotMatrix*)((intptr_t)this + this->pivotDataOffset_);
    }

    inline BasisMatrix* GetBasisMatrices() const
    {
        return (BasisMatrix*)((intptr_t)this + this->basisMatricesOffset_);
    }
};