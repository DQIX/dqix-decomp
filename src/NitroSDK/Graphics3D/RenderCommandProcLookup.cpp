#include "NitroSDK/Graphics3D/RenderCommands_Common.h"

// cursed but this has to go on its own TU
// it was probably in a TU with other functions but they all got optimized out

void BoneMatrixDataSubmissionProc_Type0(BoneMatrixRenderData*);
void BoneMatrixDataSubmissionProc_Type1(BoneMatrixRenderData*);
void BoneMatrixDataSubmissionProc_Type2(BoneMatrixRenderData*);

void BoneMatrixScaleCalculationProc_Type0(BoneMatrixRenderData*, Nitro3DBoneMatrix::Scaling*, uint8_t*, int);
void BoneMatrixScaleCalculationProc_Type1(BoneMatrixRenderData*, Nitro3DBoneMatrix::Scaling*, uint8_t*, int);
void BoneMatrixScaleCalculationProc_Type2(BoneMatrixRenderData*, Nitro3DBoneMatrix::Scaling*, uint8_t*, int);

void MaterialTextureMatrixLoadProc_Type0(MaterialRenderData* renderData);
void MaterialTextureMatrixLoadProc_Type1(MaterialRenderData* renderData);
void MaterialTextureMatrixLoadProc_Type2(MaterialRenderData* renderData);
void MaterialTextureMatrixLoadProc_Type3(MaterialRenderData* renderData);

void (*g_nitro2DPopulateBoneMatrixScalingProcs[])(BoneMatrixRenderData*, Nitro3DBoneMatrix::Scaling*, uint8_t*, int) =
{
    &BoneMatrixScaleCalculationProc_Type0,
    &BoneMatrixScaleCalculationProc_Type1,
    &BoneMatrixScaleCalculationProc_Type2
};

void (*g_nitro2DSubmitBoneMatrixProcs[])(BoneMatrixRenderData*) = 
{
    &BoneMatrixDataSubmissionProc_Type0,
    &BoneMatrixDataSubmissionProc_Type1,
    &BoneMatrixDataSubmissionProc_Type2
};

void (*g_nitro2DLoadMaterialTextureMatrixProcs[])(MaterialRenderData*) =
{
    &MaterialTextureMatrixLoadProc_Type0,
    &MaterialTextureMatrixLoadProc_Type1,
    &MaterialTextureMatrixLoadProc_Type2,
    &MaterialTextureMatrixLoadProc_Type3
};