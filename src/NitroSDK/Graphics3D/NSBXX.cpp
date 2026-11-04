#include "NitroSDK/Graphics3D/Graphics3D.h"
#include <globaldefs.h>
#include "NitroSDK/Memory/AllocatorUnion.h"

#if defined(jpn)
#define func_020b2a6c func_020b4538
#endif

extern "C"
{
    // get allocation size for animation data
    unsigned int func_020b2a6c(const void* rawAnim, Nitro3DModel* model);
}

extern "C" Nitro3DAnimation* NSBXX_Model_AllocateAnimationData(AllocatorUnion* alloc, const void* rawAnim, Nitro3DModel* model)
{
    unsigned int size = func_020b2a6c(rawAnim, model);
    return (Nitro3DAnimation*)alloc->Allocate(size);
}

extern "C" void NSBXX_Model_SetAllMaterialFlags(Nitro3DModel *model, int value, unsigned int mask)
{
    unsigned int numMaterials = model->numMaterials;
    Nitro3DModelMaterials* materialList = model->GetMaterialData();

    for (unsigned int i = 0; i < numMaterials; i++)
    {
        Nitro3DMaterial* material = materialList->GetMaterialByIndex(i);
        if (value)
            material->flags |= mask;
        else
            material->flags &= ~mask;
    }
}

// 020b6f64
extern "C" void NSBXX_Model_AdjustPolygonAttrMask(Nitro3DModel* model, int setBits, unsigned int mask)
{
    unsigned int numMaterials = model->numMaterials;
    Nitro3DModelMaterials* materialList = model->GetMaterialData();

    for (unsigned int i = 0; i < numMaterials; i++)
    {
        Nitro3DMaterial* material = materialList->GetMaterialByIndex(i);
        if (setBits)
            material->maskPOLYGON_ATTR |= mask;
        else
            material->maskPOLYGON_ATTR &= ~mask;
    }
}

// func_020b700c
// color is 15-bits in format BBBBBGGGGGRRRRR
extern "C" void NSBXX_Model_SetMaterialDiffuseReflectionColor(Nitro3DModel* model, unsigned int materialIndex, int col)
{
    Nitro3DMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    // bits 16-20: ambient red, 21-25: ambient green, 26-30: ambient blue
    material->paramDIF_AMB = material->paramDIF_AMB & 0xffff8000 | col;
}

// func_020b708c
// color is 15-bits in format BBBBBGGGGGRRRRR
extern "C" void NSBXX_Model_SetMaterialAmbientReflectionColor(Nitro3DModel* model, unsigned int materialIndex, int col)
{
    Nitro3DMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    // bits 16-20: ambient red, 21-25: ambient green, 26-30: ambient blue
    material->paramDIF_AMB = material->paramDIF_AMB & 0x8000ffff | (col << 16);
}

// 020b710c
extern "C" void NSBXX_Model_SetMaterialPolygonID(Nitro3DModel* model, unsigned int materialIndex, int id)
{
    Nitro3DMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    // set bits 24-29
    material->paramPOLYGON_ATTR = material->paramPOLYGON_ATTR & ~0x3f000000 | (id << 24);
}

extern "C" void NSBXX_Model_SetMaterialAlpha(Nitro3DModel* model, unsigned int materialIndex, int alpha)
{
    Nitro3DMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    material->paramPOLYGON_ATTR = material->paramPOLYGON_ATTR & ~0x1f0000 | (alpha << 16);
}

// 020b71fc
extern "C" int NSBXX_Model_GetMaterialPolygonID(Nitro3DModel* model, unsigned int materialIndex)
{
    Nitro3DMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    // extract bits 24-29
    return (material->paramPOLYGON_ATTR & 0x3f000000) >> 24;
}

// 020b726c
extern "C" void NSBXX_Model_SetDiffuseReflectionColor(Nitro3DModel* model, int col)
{
    for (unsigned int matIdx = 0; matIdx < model->numMaterials; matIdx++)
        NSBXX_Model_SetMaterialDiffuseReflectionColor(model, matIdx, col);
}

// 020b72ac
extern "C" void NSBXX_Model_SetAmbientReflectionColor(Nitro3DModel* model, int col)
{
    for (unsigned int matIdx = 0; matIdx < model->numMaterials; matIdx++)
        NSBXX_Model_SetMaterialAmbientReflectionColor(model, matIdx, col);
}

// 020b72ec
extern "C" void NSBXX_Model_SetPolygonID(Nitro3DModel* model, int id)
{
    for (unsigned int matIdx = 0; matIdx < model->numMaterials; matIdx++)
        NSBXX_Model_SetMaterialPolygonID(model, matIdx, id);
}

extern "C" void NSBXX_Model_SetAlpha(Nitro3DModel* model, int alpha)
{
    for (unsigned int matIdx = 0; matIdx < model->numMaterials; matIdx++)
        NSBXX_Model_SetMaterialAlpha(model, matIdx, alpha);
}