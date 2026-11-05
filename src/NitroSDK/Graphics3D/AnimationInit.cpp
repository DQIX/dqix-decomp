#include "NitroSDK/Graphics3D/Graphics3D.h"
#include "NitroSDK/Graphics3D/AnimationProcessing.h"

// We seriously have a translation unit for a single function, because the data
// is laid out in such a ridiculous way that this is the most sensible way to match it

PFNProcessMaterialAnimations g_pfnNitro3DProcessMaterialAnimations = &ProcessMaterialAnimationsOnBoundMaterial;
PFNProcessJointAnimations g_pfnNitro3DProcessJointAnimations = &ProcessJointAnimationsOnBoneMatrix;
PFNProcessVisibilityAnimations g_pfnNitro3DProcessVisibilityAnimations = &ProcessVisibilityAnimations;

PFNProcessAnimation g_pfnNitro3DProcessMAM = &MAMAnimationProcessingCallback;
PFNProcessAnimation g_pfnNitro3DProcessMPT = &MPTAnimationProcessingCallback;
PFNProcessAnimation g_pfnNitro3DProcessMAT = &MATAnimationProcessingCallback;
PFNProcessAnimation g_pfnNitro3DProcessJAC = &JACAnimationProcessingCallback;
PFNProcessAnimation g_pfnNitro3DProcessVAV = &VAVAnimationProcessingCallback;

static unsigned int s_numAnimationTypes = 5;

static struct AnimationTypeDescription
{
    Nitro3DAnimationSignature signature;
    void (*initializer)(Nitro3DAnimation* anim, void* pvMAT, Nitro3DModel* model);
} s_animationInitLookup[10] =
{
    { 'M', 0, 0x4d41, &InitializeModelAnimationFromMAM },
    { 'M', 0, 0x5450, &InitializeModelAnimationFromMPT },
    { 'M', 0, 0x5441, &InitializeModelAnimationFromMAT },
    { 'V', 0, 0x5641, &InitializeModelAnimationFromVAV },
    { 'J', 0, 0x4341, &InitializeModelAnimationFromJAC }
};

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