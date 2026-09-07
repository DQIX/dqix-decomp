#include "Graphics/NSBXX/NSBXX.h"
#include "Graphics/NSBXX/RenderCommands_Common.h"
#include "Graphics/NSBXX/Animation.h"

#pragma optimize_for_size off

#if defined(jpn)
#define func_020ca390 func_020cbe5c
#define data_020f1c80 data_020f1dec
#endif

extern "C"
{
    // memset
    void func_020ca390(int, void*, unsigned);
}

// processing callback for M.AM animations
extern void (*data_020f1c80)(void* data, AnimationData* anim, int arg);

unsigned short GetColorFromMaterialAnimation(NSBXXAnimationMAM* mam, unsigned int trackEntry, unsigned int frame)
{
    if (trackEntry & 0x20000000)
        return (unsigned short)trackEntry;

    uint16_t* samples = (uint16_t*)((intptr_t)mam + (trackEntry & 0xffff));
    if ((trackEntry & 0xc0000000) == 0)
        return samples[frame];

    unsigned int lookupIndex;

    unsigned int frameCount = (trackEntry & 0x1fff0000) >> 16;
    if (trackEntry & 0x40000000)
    {
        if (frame & 1)
        {
            if (frame > frameCount)
            {
                return *(samples + (frameCount >> 1) + 1);
            }
            lookupIndex = frame >> 1;
        }
        else
        {
            return *(samples + (frame >> 1));
        }
    }
    else 
    {
        unsigned int mod4 = frame & 3;
        if (mod4 != 0)
        {
            if (frame > frameCount)
            {
                return *(samples + (frameCount >> 2) + mod4);
            }

            if (frame & 1) // split by 1/4 and 3/4
            {
                unsigned int highWeightIdx;
                unsigned int lowWeightIdx;

                if (frame & 2)
                {
                    lowWeightIdx = frame >> 2;
                    highWeightIdx = (frame >> 2) + 1;
                }
                else
                {
                    lowWeightIdx = (frame >> 2) + 1;
                    highWeightIdx = frame >> 2;
                }
            
                unsigned int highWeight = samples[highWeightIdx];
                unsigned int lowWeight = samples[lowWeightIdx];

                unsigned int rbsum = (highWeight & 0x7c1f) * 3;
                unsigned int gsum = (highWeight & 0x3e0) * 3;

                rbsum += (lowWeight & 0x7c1f);
                gsum += (lowWeight & 0x3e0);

                return ((rbsum >> 2) & 0x7c1f) | ((gsum >> 2) & 0x3e0);
            }
            else
            {
                lookupIndex = frame >> 2;
            }
        }
        else
        {
            return samples[frame >> 2];
        }
    }

    unsigned int a = *(samples + lookupIndex);
    unsigned int b = *(samples + lookupIndex + 1);
    return (((a & 0x7c1f) + (b & 0x7c1f)) >> 1) & 0x7c1f |
        (((a & 0x3e0) + (b & 0x3e0)) >> 1) & 0x3e0;
}

unsigned short GetAlphaFromMaterialAnimation(NSBXXAnimationMAM* mam, unsigned int trackEntry, unsigned int frame)
{
    if (trackEntry & 0x20000000)
        return (unsigned short)trackEntry;

    uint8_t* samples = (uint8_t*)((intptr_t)mam + (trackEntry & 0xffff));
    if ((trackEntry & 0xc0000000) == 0)
        return samples[frame];

    unsigned int frameCount = (trackEntry & 0x1fff0000) >> 16;
    if (trackEntry & 0x40000000)
    {
        if (frame & 1)
        {
            if (frame > frameCount)
                return *(samples + (frameCount >> 1) + 1);
            return (*(samples + (frame >> 1)) + *(samples + (frame >> 1) + 1)) >> 1;
        }
        else
        {
            return samples[frame >> 1];
        }
    }
    else
    {
        if ((frame & 3) != 0)
        {
            if (frame > frameCount)
                return *(samples + (frameCount >> 2) + (frame & 3));

            if (frame & 1)
            {
                unsigned int highWeightIdx;
                unsigned int lowWeightIdx;

                if (frame & 2)
                {
                    lowWeightIdx = frame >> 2;
                    highWeightIdx = (frame >> 2) + 1;
                }
                else
                {
                    lowWeightIdx = (frame >> 2) + 1;
                    highWeightIdx = frame >> 2;
                }

                unsigned int contrib75 = samples[highWeightIdx] * 3;
                unsigned int contrib25 = samples[lowWeightIdx];
                return (contrib75 + contrib25) >> 2;
            }
            else
            {
                return (*(samples + (frame >> 2)) + *(samples + (frame >> 2) + 1)) >> 1;
            }
        }
        else
        {
            return samples[frame >> 2];
        }
    }
}

void InitializeModelAnimationFromMAM(AnimationData* anim, void* pvMAM, NSBXXInternalModel* model)
{   
    unsigned int track;
    NSBXXModelMaterialData* materials;
    if (model != NULL && model->materialsOffset_ != 0)
        materials = (NSBXXModelMaterialData*)((intptr_t)model + model->materialsOffset_);
    else
        materials = NULL;
    anim->callback_ = data_020f1c80;

    anim->numEntries_ = model->numMaterials_;
    func_020ca390(0, &anim->entries_[0], anim->numEntries_ * 2);

    NSBXXAnimationMAM* mam = (NSBXXAnimationMAM*)pvMAM;
    
    track = 0;
    if (track < mam->tracks_.numEntries_)
    {
        unsigned int bufferOffset = 0;
        do
        {   
            const char* str = mam->tracks_.GetNameByIndexAndOffset(track, bufferOffset);
            int idx;

            if (materials == NULL)
                idx = -1;
            else
                idx = NSBXXNameList_SearchIndex(&materials->materialOffsetList_, str);

            if (idx >= 0)
            {
                anim->entries_[idx] = track | 0x100;
            }
            track++;
            bufferOffset += 16;
        } while (track < mam->tracks_.numEntries_);
    }
}

void MAMAnimationProcessingCallback(void* data, AnimationData* anim, int arg)
{
    MaterialRenderData* renderData = (MaterialRenderData*)data;
    NSBXXAnimationMAM* mam = (NSBXXAnimationMAM*)anim->pRawData_;
    unsigned int frame = anim->time_ >> 12;
    NSBXXAnimationMAM::Track* track = mam->tracks_.GetEntryByIndex_notVolatile<NSBXXAnimationMAM::Track>((unsigned short)arg);

    unsigned int diffuseCol = GetColorFromMaterialAnimation(mam, track->diffuse_, frame);
    unsigned int ambientCol = GetColorFromMaterialAnimation(mam, track->ambient_, frame);

    renderData->paramDIF_AMB_ = (diffuseCol) | (ambientCol << 16) | (((renderData->paramDIF_AMB_ & 0x8000) ? 1 : 0) << 15);

    unsigned int specEmission = GetColorFromMaterialAnimation(mam, track->emission_, frame);
    unsigned int specReflection = GetColorFromMaterialAnimation(mam, track->reflection_, frame);

    renderData->paramSPE_EMI_ = (specReflection) | (specEmission << 16) | (((renderData->paramSPE_EMI_ & 0x8000) ? 1 : 0) << 15);

    unsigned int alpha = GetAlphaFromMaterialAnimation(mam, track->alpha_, frame);
    renderData->paramPOLYGON_ATTR_ = (renderData->paramPOLYGON_ATTR_ & ~0x1f0000) | (alpha << 16);
}