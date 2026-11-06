#include "NitroSDK/Graphics3D/Internal.h"

void Nitro3Di_InitAnimationFromVAV(Nitro3DAnimation* anim, void* pvVAV, Nitro3DModel* model)
{
    anim->callback = g_pfnNitro3DProcessVAV;
    anim->numEntries = model->numBoneMatrices;
    anim->pRawData = pvVAV;
    for (unsigned int index = 0; index < anim->numEntries; index++)
        anim->entries[index] = index | 0x100;
}

void Nitro3Di_ProcessAnimationVAV(void* data, Nitro3DAnimation* anim, int arg)
{
    int* outVis = (int*)data;
    Nitro3DVAV* vav = (Nitro3DVAV*)anim->pRawData;
    unsigned int frame = anim->time >> 12;
    unsigned int testIndex = frame * vav->numConditions + arg;
    *outVis = vav->bitfield[testIndex >> 5] & (1 << (testIndex & 0x1f));
}