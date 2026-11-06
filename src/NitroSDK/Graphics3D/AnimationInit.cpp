#include "NitroSDK/Graphics3D/Internal.h"

// We seriously have a translation unit for a single function, because the data
// is laid out in such a ridiculous way that this is the most sensible way to match it

PFNProcessMaterialAnimations g_pfnNitro3DProcessMaterialAnimations = &Nitro3Di_DefaultProcessMaterialAnimations;
PFNProcessJointAnimations g_pfnNitro3DProcessJointAnimations = &Nitro3Di_DefaultProcessJointAnimations;
PFNProcessVisibilityAnimations g_pfnNitro3DProcessVisibilityAnimations = &Nitro3Di_DefaultProcessVisibilityAnimations;

PFNProcessAnimation g_pfnNitro3DProcessMAM = &Nitro3Di_ProcessAnimationMAM;
PFNProcessAnimation g_pfnNitro3DProcessMPT = &Nitro3Di_ProcessAnimationMPT;
PFNProcessAnimation g_pfnNitro3DProcessMAT = &Nitro3Di_ProcessAnimationMAT;
PFNProcessAnimation g_pfnNitro3DProcessJAC = &Nitro3Di_ProcessAnimationJAC;
PFNProcessAnimation g_pfnNitro3DProcessVAV = &Nitro3Di_ProcessAnimationVAV;

static unsigned int s_numAnimationTypes = 5;

static struct AnimationTypeDescription
{
    Nitro3DAnimationSignature signature;
    void (*initializer)(Nitro3DAnimation* anim, void* pvMAT, Nitro3DModel* model);
} s_animationInitLookup[10] =
{
    { 'M', 0, 0x4d41, &Nitro3Di_InitAnimationFromMAM },
    { 'M', 0, 0x5450, &Nitro3Di_InitAnimationFromMPT },
    { 'M', 0, 0x5441, &Nitro3Di_InitAnimationFromMAT },
    { 'V', 0, 0x5641, &Nitro3Di_InitAnimationFromVAV },
    { 'J', 0, 0x4341, &Nitro3Di_InitAnimationFromJAC }
};

unsigned int Nitro3Di_GetRequiredAnimationSize(const void* animData, const Nitro3DModel* model)
{
    if (animData == NULL || model == NULL)
        return 0;
    const Nitro3DAnimationSignature* sig = (const Nitro3DAnimationSignature*)animData;
    switch (sig->signatureInitial)
    {
    case 'M':
        return (model->numMaterials * 2 + 0x1c) & ~3;
        break;
    case 'J':
    case 'V':
        return (model->numBoneMatrices * 2 + 0x1c) & ~3;
    }
    return 0;
}

void Nitro3Di_InitModelAnimation(Nitro3DAnimation *anim, void *pRawData, Nitro3DModel *model, Nitro3DTextureSet *tex)
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
    int numAnimationTypes = s_numAnimationTypes;
    for (unsigned int animType = 0; animType < numAnimationTypes; animType++)
    {
        if (signature->signatureInitial != s_animationInitLookup[animType].signature.signatureInitial ||
            signature->signatureEnd != s_animationInitLookup[animType].signature.signatureEnd)
            continue;
        
        if (s_animationInitLookup[animType].initializer != NULL)
            s_animationInitLookup[animType].initializer(anim, pRawData, model);
        break;
    }
}