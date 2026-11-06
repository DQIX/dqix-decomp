#include "NitroSDK/Graphics3D/Internal.h"

#if defined(jpn)
#define func_020ca390 func_020cbe5c
#endif

extern "C"
{
    // memset
    void func_020ca390(int, void*, unsigned);
}

static unsigned short CalculateColor(Nitro3DMAM* mam, unsigned int trackEntry, unsigned int frame)
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
                return *(samples + (frameCount >> 1) + 1);
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
                return *(samples + (frameCount >> 2) + mod4);

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
    // separate red+blue from green to prevent bit-shifts spilling into other colors
    return (((a & 0x7c1f) + (b & 0x7c1f)) >> 1) & 0x7c1f |
        (((a & 0x3e0) + (b & 0x3e0)) >> 1) & 0x3e0;
}

static unsigned short CalculateAlpha(Nitro3DMAM* mam, unsigned int trackEntry, unsigned int frame)
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

void Nitro3Di_InitAnimationFromMAM(Nitro3DAnimation* anim, void* pvMAM, Nitro3DModel* model)
{   
    
    Nitro3DModelMaterials* materials = model->GetMaterialData();
    anim->callback = g_pfnNitro3DProcessMAM;

    anim->numEntries = model->numMaterials;
    func_020ca390(0, &anim->entries[0], anim->numEntries * 2);

    Nitro3DMAM* mam = (Nitro3DMAM*)pvMAM;
    for (unsigned int track = 0; track < mam->tracks.numEntries; track++)
    {   
        const char* str = mam->tracks.GetNameByIndex(track);
        int idx;

        if (materials == NULL)
            idx = -1;
        else
            idx = Nitro3D_GetNameListIndex(&materials->materialOffsetList, str);

        if (idx >= 0)
            anim->entries[idx] = track | 0x100;
    }
}

void Nitro3Di_ProcessAnimationMAM(void* data, Nitro3DAnimation* anim, int arg)
{
    MaterialRenderData* renderData = (MaterialRenderData*)data;
    Nitro3DMAM* mam = (Nitro3DMAM*)anim->pRawData;
    unsigned int frame = anim->time >> 12;
    Nitro3DMAM::Track* track = mam->tracks.GetEntryByIndex<Nitro3DMAM::Track>((unsigned short)arg);

    unsigned int diffuseCol = CalculateColor(mam, track->diffuse, frame);
    unsigned int ambientCol = CalculateColor(mam, track->ambient, frame);

    renderData->paramDIF_AMB = (diffuseCol) | (ambientCol << 16) | (((renderData->paramDIF_AMB & 0x8000) ? 1 : 0) << 15);

    unsigned int specEmission = CalculateColor(mam, track->emission, frame);
    unsigned int specReflection = CalculateColor(mam, track->reflection, frame);

    renderData->paramSPE_EMI = (specReflection) | (specEmission << 16) | (((renderData->paramSPE_EMI & 0x8000) ? 1 : 0) << 15);

    unsigned int alpha = CalculateAlpha(mam, track->alpha, frame);
    renderData->paramPOLYGON_ATTR = (renderData->paramPOLYGON_ATTR & ~0x1f0000) | (alpha << 16);
}