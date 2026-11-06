#pragma once

#include "../NitroSDK/Graphics3D/Graphics3D.h"
#include "Memory/SafeAllocator.h"

class Model3D;

// Higher-level wrapper for NSBMD animations, interfaces with the higher-level
// Model3D instead of ModelRenderContext
class Animation3D
{
public:
    Nitro3DFileHeader* pRawFile_; // top-level file e.g. the NSBCA, NSBMA etc (decompressed)
    Nitro3DAnimation* pAnimData_;
    fix32_t playbackSpeed_;
    // bit 0: set if animation doesn't loop
    unsigned int flags_;

    void DefaultInitialize(); // might be a constructor

    bool SetRawFile(Nitro3DFileHeader* file);

    // Creates the underlying AnimationData object assuming that the pointer to the
    // raw animation file has been set.
    // If alternateModel is not null, then its TEX0 will be associated with the
    // animation instead of that of the attached model
    bool CreateData(Model3D* attachedModel, SafeAllocator* alloc, Model3D* alternateModel);

    void AdvanceTimer(fix32_t dt);

    Nitro3DAnimation* GetBasicAnimationData();
    void SetAnimationTime(fix32_t);
};