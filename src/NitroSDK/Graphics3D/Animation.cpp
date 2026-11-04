#include "NitroSDK/Graphics3D/Animation.h"

#if defined(jpn)
#define func_020ca3ec func_020cbeb8

#define data_020f1c6c data_020f1dd8
#define data_020f1c84 data_020f1df0
#define data_020f1c88 data_020f1df4
#define data_020f1c8c data_020f1df8
#define data_020f1c90 data_020f1dfc
#endif

extern "C"
{
    // memset
    void func_020ca3ec(int value, void* dst, unsigned len);
}

extern unsigned int data_020f1c6c; // number of animation types (5)

extern bool (*data_020f1c84)(int*, Nitro3DAnimation*, unsigned int);
extern bool (*data_020f1c88)(BoneMatrixRenderData*, Nitro3DAnimation*, unsigned int);
extern bool (*data_020f1c8c)(MaterialRenderData*, Nitro3DAnimation*, unsigned int);

struct AnimationTypeDescription
{
    Nitro3DAnimationSignature signature;
    void (*initializer)(Nitro3DAnimation* anim, void* pvMAT, Nitro3DModel* model);
} extern data_020f1c90[];

void InitializeModelAnimation(Nitro3DAnimation *anim, void *pRawData, Nitro3DModel *model, Nitro3DTextureSet *tex)
{
    anim->time = 0;
    anim->pRawData = pRawData;
    anim->pNext = NULL;
    anim->unknown_18 = 0x7f;
    anim->weight = 1 << 12;
    anim->pTex0 = tex;
    anim->numEntries = 0;
    anim->callback = NULL;

    Nitro3DAnimationSignature* signature = (Nitro3DAnimationSignature*)pRawData;
    int numAnimationTypes = data_020f1c6c;
    for (unsigned int animType = 0; animType < numAnimationTypes; animType++)
    {
        if (signature->signatureInitial != data_020f1c90[animType].signature.signatureInitial ||
            signature->signatureEnd != data_020f1c90[animType].signature.signatureEnd)
            continue;
        
        if (data_020f1c90[animType].initializer != NULL)
            data_020f1c90[animType].initializer(anim, pRawData, model);
        break;
    }
}

void PopulateModelRenderContext(ModelRenderContext *context, Nitro3DModel *model)
{
    func_020ca3ec(0, context, sizeof(ModelRenderContext));
    context->pfnProcessMaterialAnimations = data_020f1c8c;
    context->pfnProcessJointAnimations = data_020f1c88;
    context->pfnProcessVisibilityAnimations = data_020f1c84;
    context->model = model;
}

void AddAnimationsToList(Nitro3DAnimation** pListStart, Nitro3DAnimation* newEntry)
{
    Nitro3DAnimation* mainListEntry = *pListStart;
    
    if (mainListEntry == NULL)
    {
        *pListStart = newEntry;
        return;
    }    

    if (mainListEntry->pNext == NULL)
    {
        if (mainListEntry->unknown_18 > newEntry->unknown_18)
        {
            Nitro3DAnimation* newEntrySequenceEnd = newEntry;
            if (newEntrySequenceEnd->pNext != NULL)
            {
                do
                {
                    newEntrySequenceEnd = newEntrySequenceEnd->pNext;
                } while (newEntrySequenceEnd->pNext != NULL);
            }
            newEntrySequenceEnd->pNext = mainListEntry;
            *pListStart = newEntry;
        }
        else
        {
            mainListEntry->pNext = newEntry;
        }
        return;
    }
    Nitro3DAnimation* mainListNextEntry = mainListEntry->pNext;
    
    if (mainListEntry->pNext != NULL)
    {
        unsigned int targetOrder = newEntry->unknown_18;
        do
        {
            unsigned int nextOrder = mainListNextEntry->unknown_18;
            if (nextOrder >= targetOrder)
            {
                Nitro3DAnimation* newEntrySequenceEnd = newEntry;
                if (newEntrySequenceEnd->pNext != NULL)
                {
                    do
                    {
                        newEntrySequenceEnd = newEntrySequenceEnd->pNext;
                    } while (newEntrySequenceEnd->pNext != NULL);
                }
                mainListEntry->pNext = newEntry;
                newEntrySequenceEnd->pNext = mainListNextEntry;
                return;
            }
            mainListEntry = mainListNextEntry;
            mainListNextEntry = mainListNextEntry->pNext;
        } while (mainListEntry->pNext != NULL);
    }
    // If we get here, need to insert the new entry at the end, and
    // mainListEntry points to the final one currently in the list
    mainListEntry->pNext = newEntry;
}

// the exact same function exists in RenderCommands.cpp so we give it a slightly
// different name to distinguish it. Later can make both versions static
void PopulateBitfieldFromAnimData0(unsigned int* bitfield, Nitro3DAnimation* anim)
{
    if (anim == NULL)
        return;

    do
    {
        int counter = 0;
        if (counter < anim->numEntries)
        {
            do
            {
                if (anim->entries[counter] & 0x100)
                {
                    bitfield[counter >> 5] |= 1 << (counter & 0x1f);
                }
                counter++;
            } while (counter < anim->numEntries);
        }
        anim = anim->pNext;
    } while (anim != NULL);
}

void AddAnimationsToModelRenderContext(ModelRenderContext *context, Nitro3DAnimation *anim)
{
    if (anim == NULL || anim->pRawData == NULL)
        return;

    Nitro3DAnimationSignature* signature = (Nitro3DAnimationSignature*)anim->pRawData;

    switch (signature->signatureInitial)
    {
    case 'M':
        PopulateBitfieldFromAnimData0(context->animatedMaterials, anim);
        AddAnimationsToList(&context->materialAnimations, anim);
        break;
    case 'J':
        PopulateBitfieldFromAnimData0(context->animatedBoneMatrices, anim);
        AddAnimationsToList(&context->jointAnimations, anim);
        break;
    case 'V':
        PopulateBitfieldFromAnimData0(context->animatedVisibilityConditions, anim);
        AddAnimationsToList(&context->visibilityAnimations, anim);
        break;
    }
}

// can be made static
bool RemoveAnimationFromList(Nitro3DAnimation** pList, Nitro3DAnimation* entry)
{
    Nitro3DAnimation* firstEntry = *pList;
    if (*pList == NULL)
        return false;
    

    if (firstEntry == entry)
    {
        *pList = firstEntry->pNext;
        entry->pNext = NULL;
        return true;
    }
    
    Nitro3DAnimation* nextEntry = firstEntry->pNext;
    Nitro3DAnimation* listEntry = firstEntry;
    if (nextEntry != NULL)
    {
        do
        {
            if (nextEntry == entry)
            {
                listEntry->pNext = nextEntry->pNext;
                nextEntry->pNext = NULL;
                return true;
            }
            listEntry = nextEntry;
            nextEntry = nextEntry->pNext;
        } while (nextEntry != NULL);
    }

    return false;
}

void RemoveAnimationFromModelRenderContext(ModelRenderContext *context, Nitro3DAnimation *anim)
{
    if (RemoveAnimationFromList(&context->materialAnimations, anim) ||
        RemoveAnimationFromList(&context->jointAnimations, anim) ||
        RemoveAnimationFromList(&context->visibilityAnimations, anim))
    {
        context->flags |= (1 << 4);
    }
}

void SetModelRenderContextRenderCommandHook(ModelRenderContext *context,
    RenderCommandHook hook, int unknown, int commandID, int stage)
{
    context->renderCommandHook = hook;
    context->renderCommandHookCommandID = commandID;
    context->renderCommandHookStage = stage;
}