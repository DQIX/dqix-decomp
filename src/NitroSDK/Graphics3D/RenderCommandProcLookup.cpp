#include "NitroSDK/Graphics3D/Internal.h"

// cursed but this has to go on its own TU
// it was probably in a TU with other functions but they all got optimized out

void Nitro3Di_BoneMatrixDataSubmissionProcType0(BoneMatrixRenderData*);
void Nitro3Di_BoneMatrixDataSubmissionProcType1(BoneMatrixRenderData*);
void Nitro3Di_BoneMatrixDataSubmissionProcType2(BoneMatrixRenderData*);

void Nitro3Di_BoneMatrixScaleCalculationProcType0(BoneMatrixRenderData*, Nitro3DBoneMatrix::Scaling*, uint8_t*, int);
void Nitro3Di_BoneMatrixScaleCalculationProcType1(BoneMatrixRenderData*, Nitro3DBoneMatrix::Scaling*, uint8_t*, int);
void Nitro3Di_BoneMatrixScaleCalculationProcType2(BoneMatrixRenderData*, Nitro3DBoneMatrix::Scaling*, uint8_t*, int);

void Nitro3Di_MaterialTextureMatrixLoadProcType0(MaterialRenderData* renderData);
void Nitro3Di_MaterialTextureMatrixLoadProcType1(MaterialRenderData* renderData);
void Nitro3Di_MaterialTextureMatrixLoadProcType2(MaterialRenderData* renderData);
void Nitro3Di_MaterialTextureMatrixLoadProcType3(MaterialRenderData* renderData);

void (*g_nitro3DPopulateBoneMatrixScalingProcs[])(BoneMatrixRenderData*, Nitro3DBoneMatrix::Scaling*, uint8_t*, int) =
{
    &Nitro3Di_BoneMatrixScaleCalculationProcType0,
    &Nitro3Di_BoneMatrixScaleCalculationProcType1,
    &Nitro3Di_BoneMatrixScaleCalculationProcType2
};

void (*g_nitro3DSubmitBoneMatrixProcs[])(BoneMatrixRenderData*) = 
{
    &Nitro3Di_BoneMatrixDataSubmissionProcType0,
    &Nitro3Di_BoneMatrixDataSubmissionProcType1,
    &Nitro3Di_BoneMatrixDataSubmissionProcType2
};

void (*g_nitro3DLoadMaterialTextureMatrixProcs[])(MaterialRenderData*) =
{
    &Nitro3Di_MaterialTextureMatrixLoadProcType0,
    &Nitro3Di_MaterialTextureMatrixLoadProcType1,
    &Nitro3Di_MaterialTextureMatrixLoadProcType2,
    &Nitro3Di_MaterialTextureMatrixLoadProcType3
};