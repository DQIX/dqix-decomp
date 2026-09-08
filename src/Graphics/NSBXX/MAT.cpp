#include "Graphics/NSBXX/NSBXX.h"
#include "Graphics/NSBXX/RenderCommands_Common.h"
#include "Graphics/NSBXX/Animation.h"

#pragma optimize_for_size off

#if defined(jpn)
#define func_020ca390 func_020cbe5c
#define data_020f1c78 data_020f1de4
#endif

extern "C"
{
    // memset
    void func_020ca390(int, void*, unsigned);
}

// processing callback for M.AT animations
extern void (*data_020f1c78)(void* data, AnimationData* anim, int arg);

fix32_t SampleScalarFromMATTrack(NSBXXAnimationMAT* mat, unsigned int metadata, unsigned int offset, unsigned int frame)
{
    unsigned int lowWeightIdx;
    unsigned int highWeightIdx;
    
    if (metadata & 0x20000000)
        return offset;
    
    fix32_t* samples = (fix32_t*)((intptr_t)mat + offset);
    fix16_t* samples16 = (fix16_t*)samples;

    unsigned int averageIdx;
    if ((metadata & 0xc0000000) == 0)
    {
        goto compute_direct;
    }

{
    unsigned int endFrame = metadata & 0xffff;
    if (metadata & 0x40000000)
    {
        if (frame & 1)
        {
            if (frame > endFrame)
            {
                frame = (endFrame >> 1) + 1;
                goto compute_direct;
            }
            else
            {
                averageIdx = frame >> 1;
                goto compute_average;
            }
        }
        else
        {
            frame >>= 1;
            goto compute_direct;
        }
    }
    else
    {
        unsigned int mod4 = frame & 3;
        if (mod4 != 0)
        {
            if (frame > endFrame)
            {
                frame = mod4 + (endFrame >> 2);
                goto compute_direct;
            }
            else
            {
                if (frame & 1) // split as 3/4 and 1/4
                {
                    if (frame & 2)
                    {
                        highWeightIdx = (frame >> 2) + 1;
                        lowWeightIdx = frame >> 2;
                    }
                    else
                    {
                        highWeightIdx = frame >> 2;
                        lowWeightIdx = (frame >> 2) + 1;
                    }

                    fix32_t contrib75;
                    fix32_t contrib25;

                    if (metadata & 0x10000000)
                    {
                        contrib75 = samples16[highWeightIdx];
                        contrib25 = samples16[lowWeightIdx];
                    }
                    else
                    {
                        contrib75 = samples[highWeightIdx];
                        contrib25 = samples[lowWeightIdx];
                    }
                    contrib75 *= 3;
                    return (contrib75 + contrib25) >> 2;
                }
                else
                {
                    averageIdx = frame >> 2;
                    goto compute_average;
                }
            }
        }
        else
        {
            frame >>= 2;
            goto compute_direct;
        }
    }
}

compute_direct:
    return (metadata & 0x10000000) ? samples16[frame] : samples[frame];
compute_average:
    fix32_t sampleA, sampleB;
    if (metadata & 0x10000000)
    {
        sampleA = *(samples16 + averageIdx);
        sampleB = *(samples16 + averageIdx + 1);
    }
    else
    {
        sampleA = *(samples + averageIdx);
        sampleB = *(samples + averageIdx + 1);
    }
    return (sampleA + sampleB) >> 1;
}

uint32_t SampleSinCosFromMATTrack(NSBXXAnimationMAT* mat, unsigned int metadata, unsigned int offset, unsigned int frame)
{
    unsigned int highWeightIdx;
    unsigned int lowWeightIdx;

    if (metadata & 0x20000000)
        return offset;

    uint32_t* samples = (uint32_t*)((intptr_t)mat + offset);
    uint16_t* samples16 = (uint16_t*)samples;

    unsigned int sampleIdx;

    if ((metadata & 0xc0000000) == 0)
        goto compute_direct;

{
    unsigned int endFrame = metadata & 0xffff;
    if (metadata & 0x40000000)
    {
        if (frame & 1)
        {
            if (frame > endFrame)
            {
                frame = (endFrame >> 1) + 1;
                goto compute_direct;
            }
            else
            {
                sampleIdx = frame >> 1;
                goto compute_average;
            }
        }
        else
        {
            frame = frame >> 1;
            goto compute_direct;
        }
    }
    else
    {
        unsigned int mod4 = frame & 3;
        if (mod4 != 0)
        {
            if (frame > endFrame)
            {
                frame = mod4 + (endFrame >> 2);
                goto compute_direct;
            }
            else
            {
                if (frame & 1) // 75/25 split
                {
                    if (frame & 2)
                    {
                        highWeightIdx = (frame >> 2) + 1;
                        lowWeightIdx = frame >> 2;
                    }
                    else
                    {
                        highWeightIdx = frame >> 2;
                        lowWeightIdx = (frame >> 2) + 1;
                    }

                    fix16_t* highWeightData = (fix16_t*)(samples + highWeightIdx);
                    fix16_t* lowWeightData = (fix16_t*)(samples + lowWeightIdx);

                    int32_t sineSum = highWeightData[0] * 3;
                    sineSum += lowWeightData[0];
                    int32_t cosSum = highWeightData[1] * 3;                    
                    cosSum += lowWeightData[1];

                    return (uint16_t)(sineSum >> 2) | (cosSum >> 2) << 16;
                }
                else
                {
                    sampleIdx = frame >> 2;
                    goto compute_average;
                }
            }
        }
        else
        {
            frame = frame >> 2;
            goto compute_direct;
        }
    }
}    

compute_direct:
    return samples[frame];
compute_average:
    fix32_t avgSine = (((fix16_t*)(samples + sampleIdx))[0] + ((fix16_t*)(samples + sampleIdx))[2]) >> 1;
    fix32_t avgCosine = (((fix16_t*)(samples + sampleIdx))[1] + ((fix16_t*)(samples + sampleIdx))[3]) >> 1;

    return (uint16_t)avgSine | (avgCosine << 16);
}

void CalculateTextureTransformFromMAT(NSBXXAnimationMAT* mat, unsigned short arg, unsigned int frame, MaterialRenderData* renderData)
{
    NSBXXAnimationMAT::Track* track = mat->tracks_.GetEntryByIndex_notVolatile<NSBXXAnimationMAT::Track>(arg);
    unsigned int renderFlags = renderData->flags_;

    fix32_t tx = SampleScalarFromMATTrack(mat, track->unk_18, track->unk_1c, frame);
    fix32_t ty = SampleScalarFromMATTrack(mat, track->unk_20, track->unk_24, frame);

    if (tx == 0 && ty == 0)
    {
        renderFlags |= 4;
    }
    else
    {
        renderData->translateX_ = tx;
        renderData->translateY_ = ty;
        renderFlags &= ~4;
    }
    

    uint32_t cos_sin = SampleSinCosFromMATTrack(mat, track->unk_10, track->unk_14, frame);
    if (cos_sin == 0x10000000) // cos = 1 and sin = 0, so no rotation 
    {
        renderFlags |= 2;
    }
    else
    {
        renderData->rotationSine_ = (fix16_t)cos_sin;
        renderData->rotationCosine_ = (fix16_t)(cos_sin >> 16);
        renderFlags &= ~2;
    }

    fix32_t sx = SampleScalarFromMATTrack(mat, track->unk_0, track->unk_4, frame);
    fix32_t sy = SampleScalarFromMATTrack(mat, track->unk_8, track->unk_c, frame);

    if (sx == 0x1000 && sy == 0x1000)
    {
        renderFlags |= 1;
    }
    else
    {
        renderData->extensionScaleX_ = sx;
        renderData->extensionScaleY_ = sy;
        renderFlags &= ~1;
    }

    renderData->flags_ = renderFlags;
}

void InitializeModelAnimationFromMAT(AnimationData* anim, void* pvMAT, NSBXXInternalModel* model)
{
    unsigned int track;
    NSBXXModelMaterialData* materials;
    if (model != NULL && model->materialsOffset_ != 0)
        materials = (NSBXXModelMaterialData*)((intptr_t)model + model->materialsOffset_);
    else
        materials = NULL;
    anim->callback_ = data_020f1c78;

    anim->numEntries_ = model->numMaterials_;
    func_020ca390(0, &anim->entries_[0], anim->numEntries_ * 2);

    NSBXXAnimationMAT* mat = (NSBXXAnimationMAT*)pvMAT;
    
    track = 0;
    if (track < mat->tracks_.numEntries_)
    {
        unsigned int bufferOffset = 0;
        do
        {   
            const char* str = mat->tracks_.GetNameByIndexAndOffset(track, bufferOffset);
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
        } while (track < mat->tracks_.numEntries_);
    }
}

void MATAnimationProcessingCallback(void* data, AnimationData* anim, int arg)
{
    MaterialRenderData* renderData = (MaterialRenderData*)data;
    CalculateTextureTransformFromMAT((NSBXXAnimationMAT*)anim->pRawData_, arg, anim->time_ >> 12, renderData);
    // set texcoord transform source to 1 (texcoord source)
    renderData->paramTEXIMAGE_PARAMS_ = (renderData->paramTEXIMAGE_PARAMS_ & ~0xc0000000) | 0x40000000;
    renderData->flags_ |= 8; // use extension data
}