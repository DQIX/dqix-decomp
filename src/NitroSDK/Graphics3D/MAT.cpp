#include "NitroSDK/Graphics3D/Internal.h"

#if defined(jpn)
#define func_020ca390 func_020cbe5c
#endif

extern "C"
{
    // memset
    void func_020ca390(int, void*, unsigned);
}

static fix32_t SampleScalar(Nitro3DMAT* mat, unsigned int metadata, unsigned int offset, unsigned int frame)
{
    unsigned int lowWeightIdx;
    unsigned int highWeightIdx;
    
    if (metadata & 0x20000000)
        return offset;
    
    fix32_t* samples = (fix32_t*)((intptr_t)mat + offset);
    fix16_t* samples16 = (fix16_t*)samples;

    unsigned int averageIdx;
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

static uint32_t SampleSinCos(Nitro3DMAT* mat, unsigned int metadata, unsigned int offset, unsigned int frame)
{
    unsigned int highWeightIdx;
    unsigned int lowWeightIdx;

    if (metadata & 0x20000000)
        return offset;

    uint32_t* samples = (uint32_t*)((intptr_t)mat + offset);
    fix16_t* samples16 = (fix16_t*)samples;

    unsigned long sampleIdx;

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
    fix32_t avgSine = (samples16[2 * sampleIdx] + samples16[2 * sampleIdx + 2]) >> 1;
    fix32_t avgCosine = (samples16[2 * sampleIdx + 1] + samples16[2 * sampleIdx + 3]) >> 1;

    return (uint16_t)avgSine | (avgCosine << 16);
}

static void CalculateTextureTransform(Nitro3DMAT* mat, unsigned short arg, unsigned int frame, MaterialRenderData* renderData)
{
    Nitro3DMAT::Track* track = mat->tracks.GetEntryByIndex<Nitro3DMAT::Track>(arg);
    unsigned int renderFlags = renderData->flags;

    fix32_t tx = SampleScalar(mat, track->translateXMetadata, track->translateXOffset, frame);
    fix32_t ty = SampleScalar(mat, track->translateYMetadata, track->translateYOffset, frame);

    if (tx == 0 && ty == 0)
        renderFlags |= 4;
    else
    {
        renderData->translateX = tx;
        renderData->translateY = ty;
        renderFlags &= ~4;
    }
    

    uint32_t cos_sin = SampleSinCos(mat, track->rotationMetadata, track->rotationOffset, frame);
    if (cos_sin == 0x10000000) // cos = 1 and sin = 0, so no rotation 
        renderFlags |= 2;
    else
    {
        renderData->rotationSine = (fix16_t)cos_sin;
        renderData->rotationCosine = (fix16_t)(cos_sin >> 16);
        renderFlags &= ~2;
    }

    fix32_t sx = SampleScalar(mat, track->scaleXMetadata, track->scaleXOffset, frame);
    fix32_t sy = SampleScalar(mat, track->scaleYMetadata, track->scaleYOffset, frame);

    if (sx == 0x1000 && sy == 0x1000)
        renderFlags |= 1;
    else
    {
        renderData->extensionScaleX = sx;
        renderData->extensionScaleY = sy;
        renderFlags &= ~1;
    }

    renderData->flags = renderFlags;
}

void Nitro3Di_InitAnimationFromMAT(Nitro3DAnimation* anim, void* pvMAT, Nitro3DModel* model)
{
    
    Nitro3DModelMaterials* materials = model->GetMaterialData();
    anim->callback = g_pfnNitro3DProcessMAT;

    anim->numEntries = model->numMaterials;
    func_020ca390(0, &anim->entries[0], anim->numEntries * 2);

    Nitro3DMAT* mat = (Nitro3DMAT*)pvMAT;
    
    for (unsigned int track = 0; track < mat->tracks.numEntries; track++)
    { 
        const char* str = mat->tracks.GetNameByIndex(track);
        int idx;

        if (materials == NULL)
            idx = -1;
        else
            idx = Nitro3D_GetNameListIndex(&materials->materialOffsetList, str);

        if (idx >= 0)
            anim->entries[idx] = track | 0x100;
    }
}

void Nitro3Di_ProcessAnimationMAT(void* data, Nitro3DAnimation* anim, int arg)
{
    MaterialRenderData* renderData = (MaterialRenderData*)data;
    CalculateTextureTransform((Nitro3DMAT*)anim->pRawData, arg, anim->time >> 12, renderData);
    // set texcoord transform source to 1 (texcoord source)
    renderData->paramTEXIMAGE_PARAMS = (renderData->paramTEXIMAGE_PARAMS & ~0xc0000000) | 0x40000000;
    renderData->flags |= 8; // use extension data
}