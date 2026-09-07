#include "Graphics/NSBXX/NSBXX.h"
#include <globaldefs.h>
#include <asmhacks.h>
#include "Graphics/NSBXX/Animation.h"
#include "Memory/AllocatorUnion.h"

#if defined(jpn)
#define func_020b2a6c func_020b4538
#endif

extern "C"
{
    // get allocation size for animation data
    unsigned int func_020b2a6c(const void* rawAnim, NSBXXInternalModel* model);
}

#pragma optimize_for_size off

extern "C" AnimationData* NSBXX_Model_AllocateAnimationData(AllocatorUnion* alloc, const void* rawAnim, NSBXXInternalModel* model)
{
    unsigned int size = func_020b2a6c(rawAnim, model);
    return (AnimationData*)alloc->Allocate(size);
}

extern "C" void NSBXX_Model_SetAllMaterialFlags(NSBXXInternalModel *model, int value, unsigned int mask)
{
    unsigned int numMaterials;
    unsigned int materialIdx;
    NSBXXModelMaterialData* materialList;
    
    numMaterials = model->numMaterials_;
    if (model != NULL && model->materialsOffset_ != 0)
    {
        materialList = (NSBXXModelMaterialData*)((intptr_t)model + model->materialsOffset_);
    }
    else
        materialList = NULL;

    materialIdx = 0;
    if (materialIdx < numMaterials)
    {
        int antimask = ~mask;
        do
        {
            NSBXXMaterial* material = materialList->GetMaterialByIndex(materialIdx);
            materialIdx++;
            if (value)
                material->flags_ |= mask;
            else
                material->flags_ &= antimask;
        } while (materialIdx < numMaterials);
    }
}

// 020b6f64
extern "C" void NSBXX_Model_AdjustPolygonAttrMask(NSBXXInternalModel* model, bool setBits, unsigned int mask)
{
    unsigned int numMaterials;
    unsigned int materialIdx;
    NSBXXModelMaterialData* materialData;
    
    numMaterials = model->numMaterials_;
    if (model != NULL && model->materialsOffset_ != 0)
    {
        materialData = (NSBXXModelMaterialData*)((intptr_t)model + model->materialsOffset_);
    }
    else
        materialData = NULL;
    
    materialIdx = 0;
    if (numMaterials > UNSIGNED_ZERO())
    { 
        int antimask = ~mask;
        do
        {
            NSBXXMaterial* material = materialData->GetMaterialByIndex(materialIdx);

            if (setBits)
                material->maskPOLYGON_ATTR_ |= mask;
            else
                material->maskPOLYGON_ATTR_ &= antimask;
            materialIdx++;
        } while (materialIdx < numMaterials);
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
    unsigned int matIdx = 0;
    if (model->numMaterials_ > UNSIGNED_ZERO())
    {
        do 
        {
            NSBXX_Model_SetMaterialDiffuseReflectionColor(model, matIdx, col);
            matIdx++;
        } while (matIdx < model->numMaterials_);
    }
}

// 020b72ac
extern "C" void NSBXX_Model_SetAmbientReflectionColor(NSBXXInternalModel* model, int col)
{
    unsigned int matIdx = 0;
    if (model->numMaterials_ > UNSIGNED_ZERO())
    {
        do 
        {
            NSBXX_Model_SetMaterialAmbientReflectionColor(model, matIdx, col);
            matIdx++;
        } while (matIdx < model->numMaterials_);
    }
}

// 020b72ec
extern "C" void NSBXX_Model_SetPolygonID(NSBXXInternalModel* model, int id)
{
    unsigned int matIdx = 0;
    if (model->numMaterials_ > UNSIGNED_ZERO())
    {
        do 
        {
            NSBXX_Model_SetMaterialPolygonID(model, matIdx, id);
            matIdx++;
        } while (matIdx < model->numMaterials_);
    }
}

extern "C" void NSBXX_Model_SetAlpha(NSBXXInternalModel* model, int alpha)
{
    unsigned int materialIndex = 0;
    if (model->numMaterials_ > UNSIGNED_ZERO())
    {
        do 
        {
            NSBXX_Model_SetMaterialAlpha(model, materialIndex, alpha);
            materialIndex++;
        } while (materialIndex < model->numMaterials_);
    }
}