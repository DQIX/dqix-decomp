#include "NitroSDK/Graphics3D/Graphics3D.h"
#include "NitroSDK/Graphics3D/AnimationProcessing.h"
#include "NitroSDK/Graphics3D/RenderCommands_Common.h"

void InitializeModelAnimationFromVAV(Nitro3DAnimation* anim, void* pvVAV, Nitro3DModel* model)
{
    unsigned int index = 0;
    anim->callback = g_pfnNitro3DProcessVAV;
    anim->numEntries = model->numBoneMatrices;
    anim->pRawData = pvVAV;
    if (index < anim->numEntries)
    {
        do
        {
            anim->entries[index] = index | 0x100;
            index++;
        } while (index < anim->numEntries);
    }
}

void VAVAnimationProcessingCallback(void* data, Nitro3DAnimation* anim, int arg)
{
    int* outVis = (int*)data;
    Nitro3DVAV* vav = (Nitro3DVAV*)anim->pRawData;
    unsigned int frame = anim->time >> 12;
    unsigned int testIndex = frame * vav->numConditions + arg;
    *outVis = vav->bitfield[testIndex >> 5] & (1 << (testIndex & 0x1f));
}