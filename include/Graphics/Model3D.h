#pragma once

#include "std_library_functions.h"
#include "Vector.h"
#include "Memory/SafeAllocator.h"
#include "NSBXX/RenderCommands.h"
#include "NSBXX/Animation.h"
#include "Animation3D.h"

// sizeof == 0xac
class Model3D
{
public:
    enum TextureStagingMode
    {
        TextureStagingMode_Normal = 0,
        TextureStagingMode_HighPriority = 1,
        TextureStagingMode_Immediate = 2
    };

    ModelRenderContext renderContext_;
    NSBXXInternalModel* rawInternalModel_;
    NSBXXTex* rawTEX_;
    void* rawBMD_; // holds the (decompressed) NSBMD file
    unsigned int rawBMDFileSize_;
    // should make these six numbers a struct
    fix32_t xMax_;
    fix32_t yMax_;
    fix32_t zMax_;
    fix32_t xMin_;
    fix32_t yMin_;
    fix32_t zMin_;
    fix32_t xMiddle_;
    fix32_t maybeYBase_;
    fix32_t zMiddle_;
    fix32_t maybeApproxRadius_;
    fix32_t copyOfHeight_;
    BoneMatrixRenderData* pBoneMatrixRenderData_;
    MaterialRenderData* pMaterialRenderData_;
    int unknown_98_;
    int unknown_9c_;
    unsigned short unknown_a0_;
    short alpha_; // ranges between 0 and 31
    short imageStagingTaskID_;
    short paletteStagingTaskID_;
    int unknown_flags_a8_0_ : 1;
    int unknown_flags_a8_1_ : 1;
    int unknown_flags_a8_2_ : 1; // if 0x34, 0x38 match 0x90, 0x94 then this is set

    // the class has what seems to be a constructor (no arguments) and a
    // destructor at 0207e23c and 0207e250 (usa) respectively. But I'm
    // not including it because the compiler generates two of each which breaks
    // the build process. There's probably a way around this, maybe we can 
    // explicitly mark a symbol to not be included at link time? But for now,
    // I'm just leaving them out.
    // 
    // (For what it's worth, we know it's a constructor and destructor instead
    // of just an Init() / Destroy() pair because their pointers get passed
    // to a call to func_0200ee94, which is used to default-initialize an
    // array of non-trivially constructible objects).

    void Clear();

    void Func0207e2e0();
    void LoadFromFile(const char* path, AllocatorUnion* alloc, TextureStagingMode stagingMode);

    void CopyAndProcessRawFile(AllocatorUnion* alloc, const void* data, unsigned int len, TextureStagingMode stagingMode);
    void SetAndProcessRawFile(void* data, unsigned int len, TextureStagingMode stagingMode);

    void CopyRawFile(AllocatorUnion* alloc, const void* data, unsigned int len);
    void SetRawFile(void* rawData, unsigned int length);
    void ClearRawFileCache();
    void ProcessRawFile(TextureStagingMode stagingMode);

    bool Draw(bool applyClipping);
    // In practice this is used for shadows
    // bind is a bool. If false, no material is bound (presumably the last bound
    // material ends up being used instead)?
    bool DrawMeshWithMaterial(bool applyClipping, unsigned int materialIdx, unsigned int meshIdx, int bind);
    int TestVisible();

    int GetBoneIndex(const char* boneName);
    // Takes the TEX0 file that otherModel is pointing to and applies it
    // to this Model's MDL0
    void ApplyTexturesFromModel(Model3D* otherModel);
    NSBXXMdl* GetMDL0();
    void RemoveTextures();

    // flags & 1 : allocate bone matrix-related data
    // flags & 2 : allocate material-related data
    // flags & 4 : *don't* store pointers in the initial segment/underlying model object
    void CreateBoneMatrixAndMaterialArrays(SafeAllocator* alloc, int flags);
    void StoreBoneMatrixAndMaterialArrayPointers();

    void SetAlpha(int alpha);
    int GetAlpha() const;

    void AddAnimation(Animation3D* anim);
    void RemoveAnimations();

    void SetBoneMatrixRenderData(BoneMatrixRenderData*);
    BoneMatrixRenderData* GetBoneMatrixRenderData();

    // mark all materials to ignore their built-in diffuse color and instead
    // use the color specified in RenderConfig
    void ApplyRenderConfigMaterialDiffuseColor();

    // bits 0-4 specify red, 5-9 specify green, 10-14 specify blue
    // color is applied to all materials, except ones marked as using the
    // RenderConfig's diffuse color instead
    void SetMaterialDiffuseColor(unsigned int rgb);

    NSBXXTex* GetTEX0();
    void SetTEX0(NSBXXTex* tex);

    // Presumably you need to call this if you update the TEX
    void RestageTexturePalette();
    void RestageTextureImage();
};