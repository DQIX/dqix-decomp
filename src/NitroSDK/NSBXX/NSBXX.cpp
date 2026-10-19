#include "NitroSDK/NSBXX/NSBXX.h"
#include <globaldefs.h>
#include <asmhacks.h>
#include "NitroSDK/NSBXX/Animation.h"
#include "NitroSDK/Memory/AllocatorUnion.h"

#if defined(jpn)
#define func_020b2a6c func_020b4538
#endif

extern "C"
{
    // get allocation size for animation data
    unsigned int func_020b2a6c(const void* rawAnim, NSBXXInternalModel* model);
}

extern "C" AnimationData* NSBXX_Model_AllocateAnimationData(AllocatorUnion* alloc, const void* rawAnim, NSBXXInternalModel* model)
{
    unsigned int size = func_020b2a6c(rawAnim, model);
    return (AnimationData*)alloc->Allocate(size);
}

extern "C" void NSBXX_Model_SetAllMaterialFlags(NSBXXInternalModel *model, int value, unsigned int mask)
{
    unsigned int numMaterials = model->numMaterials_;
    NSBXXModelMaterialData* materialList = model->GetMaterialData();

    for (unsigned int i = 0; i < numMaterials; i++)
    {
        NSBXXMaterial* material = materialList->GetMaterialByIndex(i);
        if (value)
            material->flags_ |= mask;
        else
            material->flags_ &= ~mask;
    }
}

// 020b6f64
extern "C" void NSBXX_Model_AdjustPolygonAttrMask(NSBXXInternalModel* model, int setBits, unsigned int mask)
{
    unsigned int numMaterials = model->numMaterials_;
    NSBXXModelMaterialData* materialList = model->GetMaterialData();

    for (unsigned int i = 0; i < numMaterials; i++)
    {
        NSBXXMaterial* material = materialList->GetMaterialByIndex(i);
        if (setBits)
            material->maskPOLYGON_ATTR_ |= mask;
        else
            material->maskPOLYGON_ATTR_ &= ~mask;
    }
}

// func_020b700c
// color is 15-bits in format BBBBBGGGGGRRRRR
extern "C" void NSBXX_Model_SetMaterialDiffuseReflectionColor(NSBXXInternalModel* model, unsigned int materialIndex, int col)
{
    NSBXXMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    // bits 16-20: ambient red, 21-25: ambient green, 26-30: ambient blue
    material->paramDIF_AMB_ = material->paramDIF_AMB_ & 0xffff8000 | col;
}

// func_020b708c
// color is 15-bits in format BBBBBGGGGGRRRRR
extern "C" void NSBXX_Model_SetMaterialAmbientReflectionColor(NSBXXInternalModel* model, unsigned int materialIndex, int col)
{
    NSBXXMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    // bits 16-20: ambient red, 21-25: ambient green, 26-30: ambient blue
    material->paramDIF_AMB_ = material->paramDIF_AMB_ & 0x8000ffff | (col << 16);
}

// 020b710c
extern "C" void NSBXX_Model_SetMaterialPolygonID(NSBXXInternalModel* model, unsigned int materialIndex, int id)
{
    NSBXXMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    // set bits 24-29
    material->paramPOLYGON_ATTR_ = material->paramPOLYGON_ATTR_ & ~0x3f000000 | (id << 24);
}

extern "C" void NSBXX_Model_SetMaterialAlpha(NSBXXInternalModel* model, unsigned int materialIndex, int alpha)
{
    NSBXXMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    material->paramPOLYGON_ATTR_ = material->paramPOLYGON_ATTR_ & ~0x1f0000 | (alpha << 16);
}

// 020b71fc
extern "C" int NSBXX_Model_GetMaterialPolygonID(NSBXXInternalModel* model, unsigned int materialIndex)
{
    NSBXXMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    // extract bits 24-29
    return (material->paramPOLYGON_ATTR_ & 0x3f000000) >> 24;
}

// 020b726c
extern "C" void NSBXX_Model_SetDiffuseReflectionColor(NSBXXInternalModel* model, int col)
{
    for (unsigned int matIdx = 0; matIdx < model->numMaterials_; matIdx++)
        NSBXX_Model_SetMaterialDiffuseReflectionColor(model, matIdx, col);
}

// 020b72ac
extern "C" void NSBXX_Model_SetAmbientReflectionColor(NSBXXInternalModel* model, int col)
{
    for (unsigned int matIdx = 0; matIdx < model->numMaterials_; matIdx++)
        NSBXX_Model_SetMaterialAmbientReflectionColor(model, matIdx, col);
}

// 020b72ec
extern "C" void NSBXX_Model_SetPolygonID(NSBXXInternalModel* model, int id)
{
    for (unsigned int matIdx = 0; matIdx < model->numMaterials_; matIdx++)
        NSBXX_Model_SetMaterialPolygonID(model, matIdx, id);
}

extern "C" void NSBXX_Model_SetAlpha(NSBXXInternalModel* model, int alpha)
{
    for (unsigned int matIdx = 0; matIdx < model->numMaterials_; matIdx++)
        NSBXX_Model_SetMaterialAlpha(model, matIdx, alpha);
}