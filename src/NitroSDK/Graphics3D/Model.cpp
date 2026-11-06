#include "NitroSDK/Graphics3D/Graphics3D.h"
#include <globaldefs.h>
#include "NitroSDK/Memory/AllocatorUnion.h"

Nitro3DAnimation* Nitro3D_AllocateModelAnimation(AllocatorUnion* alloc, const void* rawAnim, Nitro3DModel* model)
{
    unsigned int size = Nitro3Di_GetRequiredAnimationSize(rawAnim, model);
    return (Nitro3DAnimation*)alloc->Allocate(size);
}

void Nitro3D_ConfigureModelMaterialFlags(Nitro3DModel *model, int value, unsigned int mask)
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

void Nitro3D_ConfigureModelPolygonAttrMask(Nitro3DModel* model, int setBits, unsigned int mask)
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

static void SetSingleMaterialDiffuseColor(Nitro3DModel* model, unsigned int materialIndex, int col)
{
    Nitro3DMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    // bits 16-20: ambient red, 21-25: ambient green, 26-30: ambient blue
    material->paramDIF_AMB = material->paramDIF_AMB & 0xffff8000 | col;
}

static void SetSingleMaterialAmbientReflectionColor(Nitro3DModel* model, unsigned int materialIndex, int col)
{
    Nitro3DMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    // bits 16-20: ambient red, 21-25: ambient green, 26-30: ambient blue
    material->paramDIF_AMB = material->paramDIF_AMB & 0x8000ffff | (col << 16);
}

static void SetSingleMaterialPolygonID(Nitro3DModel* model, unsigned int materialIndex, int id)
{
    Nitro3DMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    // bits 24-29
    material->paramPOLYGON_ATTR = material->paramPOLYGON_ATTR & ~0x3f000000 | (id << 24);
}

static void SetSingleMaterialAlpha(Nitro3DModel* model, unsigned int materialIndex, int alpha)
{
    Nitro3DMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    // bits 16-20
    material->paramPOLYGON_ATTR = material->paramPOLYGON_ATTR & ~0x1f0000 | (alpha << 16);
}

int Nitro3D_GetModelMaterialPolygonID(Nitro3DModel* model, unsigned int materialIndex)
{
    Nitro3DMaterial* material = model->GetMaterialData()->GetMaterialByIndex(materialIndex);
    // extract bits 24-29
    return (material->paramPOLYGON_ATTR & 0x3f000000) >> 24;
}

void Nitro3D_SetModelDiffuseColor(Nitro3DModel* model, int col)
{
    for (unsigned int matIdx = 0; matIdx < model->numMaterials; matIdx++)
        SetSingleMaterialDiffuseColor(model, matIdx, col);
}

void Nitro3D_SetModelAmbientColor(Nitro3DModel* model, int col)
{
    for (unsigned int matIdx = 0; matIdx < model->numMaterials; matIdx++)
        SetSingleMaterialAmbientReflectionColor(model, matIdx, col);
}

void Nitro3D_SetModelPolygonID(Nitro3DModel* model, int id)
{
    for (unsigned int matIdx = 0; matIdx < model->numMaterials; matIdx++)
        SetSingleMaterialPolygonID(model, matIdx, id);
}

void Nitro3D_SetModelAlpha(Nitro3DModel* model, int alpha)
{
    for (unsigned int matIdx = 0; matIdx < model->numMaterials; matIdx++)
        SetSingleMaterialAlpha(model, matIdx, alpha);
}