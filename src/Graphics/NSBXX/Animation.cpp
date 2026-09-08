#include "Graphics/NSBXX/Animation.h"

#pragma optimize_for_size off

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

extern bool (*data_020f1c84)(int*, AnimationData*, unsigned int);
extern bool (*data_020f1c88)(BoneMatrixRenderData*, AnimationData*, unsigned int);
extern bool (*data_020f1c8c)(MaterialRenderData*, AnimationData*, unsigned int);

struct AnimationTypeDescription
{
    NSBXXAnimationSignature signature;
    void (*initializer)(AnimationData* anim, void* pvMAT, NSBXXInternalModel* model);
} extern data_020f1c90[];

void InitializeModelAnimation(AnimationData *anim, void *pRawData, NSBXXInternalModel *model, NSBXXTex *tex)
{
    unsigned int animType = 0;
    anim->time_ = 0;
    anim->pRawData_ = pRawData;
    anim->pNext_ = NULL;
    anim->unk_18 = 0x7f;
    anim->weight_ = 1 << 12;
    anim->pTex0_ = tex;
    anim->numEntries_ = 0;
    anim->callback_ = NULL;

    NSBXXAnimationSignature* signature = (NSBXXAnimationSignature*)pRawData;

    unsigned int numAnimationTypes = data_020f1c6c;
    if (animType < numAnimationTypes)
    {
        int thisInitial = signature->signatureInitial_;
        while (true)
        {
            NSBXXAnimationSignature* targetSig = &data_020f1c90[animType].signature;
            int targetInitial = targetSig->signatureInitial_;
            
            if (thisInitial == targetInitial)
            {
                // annoying but necessary for matching assembly
                NSBXXAnimationSignature* foo = (NSBXXAnimationSignature*)((intptr_t)data_020f1c90 + animType * sizeof(AnimationTypeDescription));
                int targetEnd = foo->signatureEnd_;
                if (signature->signatureEnd_ == targetEnd)
                {
                    if (data_020f1c90[animType].initializer != NULL)
                        data_020f1c90[animType].initializer(anim, pRawData, model);
                    return;
                }
            }
            animType++;
            if (animType >= numAnimationTypes)
                return;
        }
    }
}

void PopulateModelRenderContext(ModelRenderContext *context, NSBXXInternalModel *model)
{
    func_020ca3ec(0, context, sizeof(ModelRenderContext));
    context->pfnProcessMaterialAnimations_ = data_020f1c8c;
    context->pfnProcessJointAnimations_ = data_020f1c88;
    context->pfnProcessVisibilityAnimations_ = data_020f1c84;
    context->internalModel_ = model;
}

void AddAnimationsToList(AnimationData** pListStart, AnimationData* newEntry)
{
    AnimationData* mainListEntry = *pListStart;
    
    if (mainListEntry == NULL)
    {
        *pListStart = newEntry;
        return;
    }    

    if (mainListEntry->pNext_ == NULL)
    {
        if (mainListEntry->unk_18 > newEntry->unk_18)
        {
            AnimationData* newEntrySequenceEnd = newEntry;
            if (newEntrySequenceEnd->pNext_ != NULL)
            {
                do
                {
                    newEntrySequenceEnd = newEntrySequenceEnd->pNext_;
                } while (newEntrySequenceEnd->pNext_ != NULL);
            }
            newEntrySequenceEnd->pNext_ = mainListEntry;
            *pListStart = newEntry;
        }
        else
        {
            mainListEntry->pNext_ = newEntry;
        }
        return;
    }
    AnimationData* mainListNextEntry = mainListEntry->pNext_;
    
    if (mainListEntry->pNext_ != NULL)
    {
        unsigned int targetOrder = newEntry->unk_18;
        do
        {
            unsigned int nextOrder = mainListNextEntry->unk_18;
            if (nextOrder >= targetOrder)
            {
                AnimationData* newEntrySequenceEnd = newEntry;
                if (newEntrySequenceEnd->pNext_ != NULL)
                {
                    do
                    {
                        newEntrySequenceEnd = newEntrySequenceEnd->pNext_;
                    } while (newEntrySequenceEnd->pNext_ != NULL);
                }
                mainListEntry->pNext_ = newEntry;
                newEntrySequenceEnd->pNext_ = mainListNextEntry;
                return;
            }
            mainListEntry = mainListNextEntry;
            mainListNextEntry = mainListNextEntry->pNext_;
        } while (mainListEntry->pNext_ != NULL);
    }
    // If we get here, need to insert the new entry at the end, and
    // mainListEntry points to the final one currently in the list
    mainListEntry->pNext_ = newEntry;
}

// the exact same function exists in RenderCommands.cpp so we give it a slightly
// different name to distinguish it. Later can make both versions static
void PopulateBitfieldFromAnimData0(unsigned int* bitfield, AnimationData* anim)
{
    if (anim == NULL)
        return;

    do
    {
        int counter = 0;
        if (counter < anim->numEntries_)
        {
            do
            {
                if (anim->entries_[counter] & 0x100)
                {
                    bitfield[counter >> 5] |= 1 << (counter & 0x1f);
                }
                counter++;
            } while (counter < anim->numEntries_);
        }
        anim = anim->pNext_;
    } while (anim != NULL);
}

void AddAnimationsToModelRenderContext(ModelRenderContext *context, AnimationData *anim)
{
    if (anim == NULL || anim->pRawData_ == NULL)
        return;

    NSBXXAnimationSignature* signature = (NSBXXAnimationSignature*)anim->pRawData_;

    switch (signature->signatureInitial_)
    {
    case 'M':
        PopulateBitfieldFromAnimData0(context->animatedMaterials_, anim);
        AddAnimationsToList(&context->materialAnimations_, anim);
        break;
    case 'J':
        PopulateBitfieldFromAnimData0(context->animatedBoneMatrices_, anim);
        AddAnimationsToList(&context->jointAnimations_, anim);
        break;
    case 'V':
        PopulateBitfieldFromAnimData0(context->animatedVisibilityConditions_, anim);
        AddAnimationsToList(&context->visibilityAnimations_, anim);
        break;
    }
}

// can be made static
bool RemoveAnimationFromList(AnimationData** pList, AnimationData* entry)
{
    AnimationData* firstEntry = *pList;
    if (*pList == NULL)
        return false;
    

    if (firstEntry == entry)
    {
        *pList = firstEntry->pNext_;
        entry->pNext_ = NULL;
        return true;
    }
    
    AnimationData* nextEntry = firstEntry->pNext_;
    AnimationData* listEntry = firstEntry;
    if (nextEntry != NULL)
    {
        do
        {
            if (nextEntry == entry)
            {
                listEntry->pNext_ = nextEntry->pNext_;
                nextEntry->pNext_ = NULL;
                return true;
            }
            listEntry = nextEntry;
            nextEntry = nextEntry->pNext_;
        } while (nextEntry != NULL);
    }

    return false;
}

void RemoveAnimationFromModelRenderContext(ModelRenderContext *context, AnimationData *anim)
{
    if (RemoveAnimationFromList(&context->materialAnimations_, anim) ||
        RemoveAnimationFromList(&context->jointAnimations_, anim) ||
        RemoveAnimationFromList(&context->visibilityAnimations_, anim))
    {
        context->flags_ |= (1 << 4);
    }
}

void SetModelRenderContextRenderCommandHook(ModelRenderContext *context,
    RenderCommandHook hook, int unknown, int commandID, int stage)
{
    context->renderCommandHook_ = hook;
    context->renderCommandHookCommandID_ = commandID;
    context->renderCommandHookStage_ = stage;
}