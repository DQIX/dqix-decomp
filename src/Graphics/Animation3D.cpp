#include "Graphics/Animation3D.h"
#include "Graphics/Model3D.h"

fix32_t GetAnimationFrameCountFix32(Nitro3DAnimation*);

#define ANIMATION_FLAG_NOT_LOOPING 0

void Animation3D::DefaultInitialize()
{
    pRawFile_ = NULL;
    pAnimData_ = NULL;
    playbackSpeed_ = 1 << 12;
    flags_ = 0;
}

bool Animation3D::SetRawFile(Nitro3DFileHeader* file)
{
    if (file == NULL)
        return false;
    pRawFile_ = file;
    return true;
}

bool Animation3D::CreateData(Model3D *attachedModel, SafeAllocator *alloc, Model3D *alternateModel)
{
    if (pRawFile_ == NULL)
        return false;

    if (attachedModel == NULL)
        return false;

    if (alloc == NULL)
        return false;

    Model3D* modelIfValid = (attachedModel->unknown_flags_a8_0_) ? attachedModel : NULL;
    Nitro3DTextureSet* tex0;
    Nitro3DModel* internalModel = attachedModel->rawInternalModel_;

    tex0 = attachedModel->GetTEX0();
    if (alternateModel != NULL)
        tex0 = alternateModel->GetTEX0();

    if (modelIfValid == NULL)
        return false;

    if (internalModel == NULL)
        return false;

    void* rawInnerAnim = Nitro3D_GetInnerObjectByIndex(pRawFile_, 0);
    if (rawInnerAnim == NULL)
        return false;

    Nitro3DAnimation* allocatedData = Nitro3D_AllocateModelAnimation(&alloc->allocUnion, rawInnerAnim, internalModel);
    if (allocatedData == NULL)
        return false;

    Nitro3Di_InitModelAnimation(allocatedData, rawInnerAnim, internalModel, tex0);
    pAnimData_ = allocatedData;
    return true;
}

void Animation3D::AdvanceTimer(fix32_t dt)
{
    if (!(pAnimData_ != NULL ? 1 : 0))
        return;

    int64_t prod = (int64_t)dt;
    prod = ((playbackSpeed_ * prod) + 0x800) >> 12;
    pAnimData_->time += (fix32_t)prod;
    if (GetAnimationFrameCountFix32(pAnimData_) <= pAnimData_->time)
    {
        if (flags_ & (1 << ANIMATION_FLAG_NOT_LOOPING))
        {
            pAnimData_->time = GetAnimationFrameCountFix32(pAnimData_);
        }
        else
        {
            while (GetAnimationFrameCountFix32(pAnimData_) <= pAnimData_->time)
            {
                pAnimData_->time -= GetAnimationFrameCountFix32(pAnimData_);
            }
            if (pAnimData_->time < 0)
                pAnimData_->time = 0;
        }
    }
}

Nitro3DAnimation* Animation3D::GetBasicAnimationData() { return pAnimData_; }

void Animation3D::SetAnimationTime(fix32_t to)
{
    if (pAnimData_ != NULL)
        pAnimData_->time = to;
}