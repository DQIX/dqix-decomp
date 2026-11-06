#pragma once

#include "../Matrix.h"
#include "Common.h"
#include "BoneMatrix.h"

struct Nitro3DRenderCommandHandler;
typedef void (*RenderCommandHook)(Nitro3DRenderCommandHandler*);

struct BoneMatrixRenderData
{
    union Scale
    {
        struct {
            fix32_t x; fix32_t y; fix32_t z;
        };
        fix32_t array[3];
    };
    // bit 0: set if no scaling
    // bit 1: set if no rotation
    // bit 2: set if no translation
    // if bit 3 is set, the bone matrix calculation callback treats scale_v1
    // as the identity, and if bit 4 is set, it treats scale_v2 as the identity
    unsigned int flags;
    Scale scale_v0;
    Scale scale_v1;
    Scale scale_v2;
    Matrix3x3 rotationMatrix;
    Vector3fix translate;
};

struct MaterialRenderData
{
    // bit 0: material does not have type 1 (scaling) extension data
    // bit 1: material does not have type 2 (rotation) extension data
    // bit 2: material does not have type 3 (translation) extension data
    // bit 3: material *has* any of type 1-3 extension data
    // bit 4: ???, ORed with bit 3 in some places, notably to call
    //        callback_f0 in the handler
    // bit 5: if set, alpha = 0 when drawing
    unsigned int flags;
    unsigned int paramDIF_AMB;
    unsigned int paramSPE_EMI;
    unsigned int paramPOLYGON_ATTR;
    unsigned int paramTEXIMAGE_PARAMS;
    unsigned int texturePaletteBase;
    fix32_t extensionScaleX;
    fix32_t extensionScaleY;
    fix16_t rotationSine;
    fix16_t rotationCosine;
    fix32_t translateX;
    fix32_t translateY;
    unsigned short materialWidth;
    unsigned short materialHeight;
    fix32_t materialScaleX;
    fix32_t materialScaleY;
};

struct Nitro3DAnimation;
struct Nitro3DModel;
struct Nitro3DModelMaterials;
struct Nitro3DTextureSet;

// sizeof == 0x54
struct Nitro3DRenderContext
{
    // bits 0, 1, 2, 3 set flags 7, 8, 9, 10 in RenderCommandHandler
    // bit 4: animation bitfields dirty
    unsigned int flags;
    Nitro3DModel* model;
    Nitro3DAnimation* materialAnimations; // M.AM, M.AT and M.PT animations
    // points to ProcessMaterialAnimationsOnBoundMaterial (020b3ad0 usa)
    bool (*pfnProcessMaterialAnimations)(MaterialRenderData*, Nitro3DAnimation*, unsigned int);
    Nitro3DAnimation* jointAnimations; // J.AC animations
    // points to ProcessJointAnimationsOnBoneMatrix (020b3bac usa)
    bool (*pfnProcessJointAnimations)(BoneMatrixRenderData*, Nitro3DAnimation*, unsigned int);
    Nitro3DAnimation* visibilityAnimations; // V.AV animations (never used?)
    // points to ProcessVisibilityAnimations (020b3f98 usa)
    bool (*pfnProcessVisibilityAnimations)(int*, Nitro3DAnimation*, unsigned int);
    void (*renderCommandHook)(Nitro3DRenderCommandHandler*);
    unsigned char renderCommandHookCommandID;
    unsigned char renderCommandHookStage; // 1,2 or 3 depending on when in the function it gets called
    char pad_26[2];
    void (*preRenderCallback)(Nitro3DRenderCommandHandler*);
    char unk_2c[4];
    uint8_t* renderCommandList;
    BoneMatrixRenderData* boneMatrixRenderDataArray;
    MaterialRenderData* materialRenderDataArray;
    // packed array of bools, where the k'th entry is given by the
    // (k & 0x1f)'th bit of array[k >> 5]. The index refers to a material
    // (matching the Nitro3DModel's MaterialList) or bone matrix (not 
    // sure about visibility animations), and pfnProcess...Animations will
    // only run for a particular object if its bit is set here.
    unsigned int animatedMaterials[2];
    unsigned int animatedBoneMatrices[2];
    unsigned int animatedVisibilityConditions[2];
};

// sizeof == 0x188
struct Nitro3DRenderCommandHandler
{
    uint8_t* instructionPointer;
    Nitro3DRenderContext* context;
    unsigned int flags;
    RenderCommandHook hooks[32];
    unsigned char hookStages[32];
    unsigned char command2Arg1;
    unsigned char boundMaterial;
    unsigned char currentBoneMatrix;
    char pad_af;
    MaterialRenderData* pMaterialRenderData;
    BoneMatrixRenderData* pBoneMatrixRenderData;
    int* pCommand2Word;
    unsigned int materialBitfield[2];
    // If bit n is set (0 <= n <= 63), then the bone of index n has
    // no non-trivial scaling
    unsigned int boneMatrixBitfield[2];
    unsigned int invBindBitfield[2];
    Nitro3DNameList* boneList;
    Nitro3DModelMaterials* modelMaterials;
    Nitro3DNameList* meshList;
    fix32_t upScale;
    fix32_t downScale;
    // specifically seems to populate the scaling part
    void (*pfnBoneMatrixRenderDataScalePopulate)(BoneMatrixRenderData*, Nitro3DBoneMatrix::Scaling* boneMatrixScaleData, uint8_t* ip, int boneMatrixFlags);
    void (*pfnBoneMatrixRenderDataSubmit)(BoneMatrixRenderData*);
    void (*pfnTextureMatrixCreate)(MaterialRenderData*);
    MaterialRenderData scratchMaterialRenderData;
    BoneMatrixRenderData scratchBoneMatrixRenderData;
    int scratchCommand2Word;
};

// acts as a gate for commands 3, 5, 12 and 13, and sometimes 4: if not set,
// the command will be skipped
#define RCH_FLAG_0 0
// set if the bound material has alpha = 0
#define RCH_FLAG_1 1
// set when command 2 runs, never checked or cleared
#define RCH_FLAG_2 2
// set if there's a material bound
#define RCH_FLAG_3 3
// set by command 6. I don't see it being cleared anywhere though?
// Some functions that hook command 6 (see Object3D's bone tracking mechanism)
// check this before doing their operations
#define RCH_FLAG_4 4
// execution done (hit the 1 opcode)
#define RCH_FLAG_5 5
// cleared before calling every hook, if then set by the hook the subsequent
// code will not execute
#define RCH_FLAG_6 6
// relates to material/bone matrix render data in the Nitro3DModel somehow
#define RCH_FLAG_7 7
// if set, most commands have their main operations skipped, but
// callbacks/hooks all happen as normal
#define RCH_FLAG_8 8
// if set, all commands other than 0, 1, 6, 9, 10 are skipped
#define RCH_FLAG_9 9
// probably something like 'no bone matrices' given that it forces command
// 6 to return early
#define RCH_FLAG_10 10

void Nitro3D_Render(Nitro3DRenderContext* renderData);
// much more barebones implementation of rendering and just does a single
// mesh / material pair. In practice this seems to be used for shadows
// bindMaterial is really a bool
void Nitro3D_RenderMeshWithMaterial(Nitro3DModel* model, unsigned int materialIdx, unsigned int meshIdx, int bindMaterial);