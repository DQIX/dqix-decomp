#include "NitroSDK/Graphics3D/Graphics3D.h"
#include "NitroSDK/Graphics3D/RenderCommands_Common.h"

#if defined(jpn)
#define data_020f1c70 data_020f1ddc
#endif

// processing callback for V.AV animations
extern void (*data_020f1c70)(void* data, Nitro3DAnimation* anim, int arg);

void InitializeModelAnimationFromVAV(Nitro3DAnimation* anim, void* pvVAV, Nitro3DModel* model)
{
    unsigned int index = 0;
    anim->callback = data_020f1c70;
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