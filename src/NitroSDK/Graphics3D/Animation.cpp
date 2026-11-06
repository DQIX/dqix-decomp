#include "NitroSDK/Graphics3D/Internal.h"

#if defined(jpn)
#define func_020ca3ec func_020cbeb8
#endif

extern "C"
{
    // memset
    void func_020ca3ec(int value, void* dst, unsigned len);
}

void Nitro3D_PopulateRenderContext(Nitro3DRenderContext *context, Nitro3DModel *model)
{
    func_020ca3ec(0, context, sizeof(Nitro3DRenderContext));
    context->pfnProcessMaterialAnimations = g_pfnNitro3DProcessMaterialAnimations;
    context->pfnProcessJointAnimations = g_pfnNitro3DProcessJointAnimations;
    context->pfnProcessVisibilityAnimations = g_pfnNitro3DProcessVisibilityAnimations;
    context->model = model;
}

static void AddAnimationToList(Nitro3DAnimation** pListStart, Nitro3DAnimation* newEntry)
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
            while (newEntrySequenceEnd->pNext != NULL)
                newEntrySequenceEnd = newEntrySequenceEnd->pNext;
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
    
    while (mainListEntry->pNext != NULL)
    {
        if (mainListNextEntry->unknown_18 >= newEntry->unknown_18)
        {
            Nitro3DAnimation* newEntrySequenceEnd = newEntry;
            while (newEntrySequenceEnd->pNext != NULL)
                newEntrySequenceEnd = newEntrySequenceEnd->pNext;
            mainListEntry->pNext = newEntry;
            newEntrySequenceEnd->pNext = mainListNextEntry;
            return;
        }
        mainListEntry = mainListNextEntry;
        mainListNextEntry = mainListNextEntry->pNext;
    }
    // If we get here, need to insert the new entry at the end, and
    // mainListEntry points to the final one currently in the list
    mainListEntry->pNext = newEntry;
}

// the exact same function exists in RenderCommands.cpp so we give it a slightly
// different name to distinguish it. Later can make both versions static
static void PopulateBitfieldFromAnimData(unsigned int* bitfield, Nitro3DAnimation* anim)
{
    while (anim != NULL)
    {
        for (int i = 0; i < anim->numEntries; i++)
        {
            if (anim->entries[i] & 0x100)
                bitfield[i >> 5] |= 1 << (i & 0x1f);
        }
        anim = anim->pNext;
    }
}

void Nitro3D_AddAnimationToContext(Nitro3DRenderContext *context, Nitro3DAnimation *anim)
{
    if (anim == NULL || anim->pRawData == NULL)
        return;

    Nitro3DAnimationSignature* signature = (Nitro3DAnimationSignature*)anim->pRawData;

    switch (signature->signatureInitial)
    {
    case 'M':
        PopulateBitfieldFromAnimData(context->animatedMaterials, anim);
        AddAnimationToList(&context->materialAnimations, anim);
        break;
    case 'J':
        PopulateBitfieldFromAnimData(context->animatedBoneMatrices, anim);
        AddAnimationToList(&context->jointAnimations, anim);
        break;
    case 'V':
        PopulateBitfieldFromAnimData(context->animatedVisibilityConditions, anim);
        AddAnimationToList(&context->visibilityAnimations, anim);
        break;
    }
}

static bool RemoveAnimationFromList(Nitro3DAnimation** pList, Nitro3DAnimation* entry)
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
    while (nextEntry != NULL)
    {
        if (nextEntry == entry)
        {
            listEntry->pNext = nextEntry->pNext;
            nextEntry->pNext = NULL;
            return true;
        }
        listEntry = nextEntry;
        nextEntry = nextEntry->pNext;
    }

    return false;
}

void Nitro3D_RemoveAnimationFromContext(Nitro3DRenderContext *context, Nitro3DAnimation *anim)
{
    if (RemoveAnimationFromList(&context->materialAnimations, anim) ||
        RemoveAnimationFromList(&context->jointAnimations, anim) ||
        RemoveAnimationFromList(&context->visibilityAnimations, anim))
    {
        context->flags |= (1 << 4);
    }
}

void Nitro3D_SetRenderHook(Nitro3DRenderContext *context,
    RenderCommandHook hook, int unknown, int commandID, int stage)
{
    context->renderCommandHook = hook;
    context->renderCommandHookCommandID = commandID;
    context->renderCommandHookStage = stage;
}