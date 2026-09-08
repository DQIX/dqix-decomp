#include "Graphics/NSBXX/NSBXX.h"
#include "Graphics/NSBXX/RenderCommands_Common.h"
#include "Graphics/NSBXX/Animation.h"

#pragma optimize_for_size off

#if defined(jpn)
#define data_020f1c70 data_020f1ddc
#endif

// processing callback for V.AV animations
extern void (*data_020f1c70)(void* data, AnimationData* anim, int arg);

void InitializeModelAnimationFromVAV(AnimationData* anim, void* pvVAV, NSBXXInternalModel* model)
{
    unsigned int index = 0;
    anim->callback_ = data_020f1c70;
    anim->numEntries_ = model->numBoneMatrices_;
    anim->pRawData_ = pvVAV;
    if (index < anim->numEntries_)
    {
        do
        {
            anim->entries_[index] = index | 0x100;
            index++;
        } while (index < anim->numEntries_);
    }
}

void VAVAnimationProcessingCallback(void* data, AnimationData* anim, int arg)
{
    int* outVis = (int*)data;
    NSBXXAnimationVAV* vav = (NSBXXAnimationVAV*)anim->pRawData_;
    unsigned int frame = anim->time_ >> 12;
    unsigned int testIndex = frame * vav->numConditions_ + arg;
    *outVis = vav->bitfield_[testIndex >> 5] & (1 << (testIndex & 0x1f));
}