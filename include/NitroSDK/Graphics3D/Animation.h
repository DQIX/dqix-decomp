#pragma once

#include "RenderCommands.h"

struct Nitro3DAnimationSignature
{
    // holds e.g. "J\0AC" or "M\0PT" but in practice is read
    // using a uint8_t and a uint16_t
    uint8_t signatureInitial;
    uint8_t signatureNull;
    uint16_t signatureEnd;
};

// Used to reference some kind of animation, specifically the innermost part
// of the NSBxx files. For example in a .nsbca file, the header has signature
// BCA0, the first file within this has signature JNT0, and the objects within
// that have signature J.AC (. = 0x00), and this struct references the J.AC object.
// Also known to be used for M.AM, M.AT, M.PT. Theoretically usable for something
// of form V.AV but it doesn't seem to be used anywhere in practice.
// Note: the V.?? is completely undocumented as far as I know, and the fact it's
// called V.AV just comes from looking at 0x020f1ca8 in the binary (USA)
struct Nitro3DAnimation
{
    typedef void (*Callback)(void*, Nitro3DAnimation*, int);

    fix32_t time;
    // used for e.g. combining effects of multiple bones?
    fix32_t weight;
    void* pRawData;
    // call this to populate renderData as appropriate
    void (*callback)(void* renderData, Nitro3DAnimation* animData, int arg);
    Nitro3DAnimation* pNext;
    Nitro3DTextureSet* pTex0; // we always supply this, but only M.PT animations use it
    unsigned char unknown_18;
    unsigned char numEntries;
    // Not sure about max array length.
    // Each entry combines the following values:
    // bits 0-7: which animation track to use for the relevant thing (material,
    // bone matrix etc). That is, entries[x] = y if track #y in the animation
    // pertains to {material, bone matrix, ...} #x in the model. However, there
    // seems to be a bug with the J.AC functionality: the function populating
    // this struct (InitializeModelAnimationFromJAC, 020b7840 usa) fills the
    // list with entries[y] = x instead. All other usage of the array is consistent
    // with the previous description. In practice we seem to always have x = y
    // and so this never becomes an issue.
    // bit 8: bool indicating whether this is 'used' / relevant in some way - 
    // the ModelRenderData bitfields get populated based on this
    // bit 9: ???, sometimes tested in conjunction with bit 8 (often things
    // only run if entry & 0x300 == 0x100)
    unsigned short entries[64];
};


unsigned int Nitro3Di_GetRequiredAnimationSize(const void* animData, const Nitro3DModel* model);
void Nitro3Di_InitModelAnimation(Nitro3DAnimation* anim, void* pRawData, Nitro3DModel* model, Nitro3DTextureSet* tex);
void Nitro3D_PopulateRenderContext(Nitro3DRenderContext* context, Nitro3DModel* model);

// Only the first animation in the list is checked to determine where to add it,
// so maybe it's assumed that you only add one? But in theory you can add
// multiple of the same type if they're already chained together
void Nitro3D_AddAnimationToContext(Nitro3DRenderContext* context, Nitro3DAnimation* anim);

void Nitro3D_RemoveAnimationFromContext(Nitro3DRenderContext* context, Nitro3DAnimation* anim);

// really doesn't fit here...
void Nitro3D_SetRenderHook(Nitro3DRenderContext* context, RenderCommandHook hook, int unknown, int commandID, int stage);

bool Nitro3Di_DefaultProcessMaterialAnimations(MaterialRenderData* material, Nitro3DAnimation* anim, unsigned int matIdx);
bool Nitro3Di_DefaultProcessJointAnimations(BoneMatrixRenderData* bone, Nitro3DAnimation* anim, unsigned int boneIdx);
bool Nitro3Di_ProcessVisibilityAnimations(int* output, Nitro3DAnimation* anim, unsigned int boneIdx);