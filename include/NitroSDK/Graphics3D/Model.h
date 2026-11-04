#pragma once

#include "Common.h"

struct Nitro3DBoundingBox
{
    // all quantities are 16-bit fixed points.
    // scurest nsbmd docs say last 3 are xMax etc but that's not how
    // they're used in functions here
    fix16_t xMin_;
    fix16_t yMin_;
    fix16_t zMin_;
    fix16_t xSize_;
    fix16_t ySize_;
    fix16_t zSize_;
};

struct Nitro3DMesh
{
    uint16_t unk_0;
    uint16_t size_;
    uint32_t unk_4;
    uint32_t gpuCommandsOffset_;
    uint32_t gpuCommandsLength_; // length in bytes

    inline const uint32_t* GetGPUCommands() const
    {
        return (uint32_t*)((intptr_t)this + gpuCommandsOffset_);
    }
};

struct Nitro3DInvBindMatrix
{
    Matrix4x3 mat4x3;
    Matrix3x3 mat3x3;
};

struct Nitro3DModel
{
    uint32_t filesize_;
    uint32_t renderCommandsOffset_;
    uint32_t materialsOffset_;
    uint32_t meshesOffset_; // offset to a NameList of NSBXXMesh
    uint32_t inverseBindsOffset_;
    uint8_t unk_14;
    // 0, 1 or 2, determines the behavior of applying scaling
    // to bones. 0 = simplest, only have one scaling operation followed
    // by rotation and translation.
    // In modes 1 and 2, each bone also provides a scale factor to be
    // used by its children.
    // Might have something to do with 'segment scale compensate'
    // behaviour? https://download.autodesk.com/us/maya/2011help/nodes/joint.html
    uint8_t boneScalingMode_;
    uint8_t materialCallbackType_;
    uint8_t numBoneMatrices_;
    uint8_t numMaterials_;
    uint8_t numMeshes_;
    uint8_t unk_1a[2];
    fix32_t upScale_; // 32-bit fixed point
    fix32_t downScale_; // 32-bit fixed point
    uint16_t numVertices_;
    uint16_t numPolygons_;
    uint16_t numTriangles_;
    uint16_t numQuads_;
    Nitro3DBoundingBox bounds_;
    int32_t maybeScale_;
    char unk_3c[4];
    Nitro3DNameList boneList_;

    // using this seems to screw up register assignment, but it's here in case
    // we can make it work later somehow
    inline Nitro3DModelMaterials* GetMaterialData() const
    {
        if (this != NULL && materialsOffset_ != 0)
            return (Nitro3DModelMaterials*)((intptr_t)this + materialsOffset_);
        else
            return NULL;
    }

    inline Nitro3DNameList* GetMeshListUnsafe() const
    {
        return (Nitro3DNameList*)((intptr_t)this + meshesOffset_);
    }

    inline Nitro3DNameList* GetMeshListSafe() const
    {
        return (this != NULL && meshesOffset_ != 0) ? GetMeshListUnsafe() : NULL;
    }

    inline Nitro3DNameList* GetMeshList() const
    {
        if (this != NULL && meshesOffset_ != 0)
            return (Nitro3DNameList*)((intptr_t)this + meshesOffset_);
        return NULL;
    }
};

// represents an MDL0 subfile in an NSBMD
struct Nitro3DModelSet
{
    uint32_t signature_;
    uint32_t unknown_4;
    Nitro3DNameList nameList_;

    Nitro3DModel* GetInternalModelByIndex(unsigned int n) const
    {
        Nitro3DModel* internalModel;
        if (this != NULL)
        {
            uint32_t* pModelOffset = nameList_.GetEntryByIndex<uint32_t>(n);

            if (pModelOffset != NULL)
            {
                internalModel = (Nitro3DModel*)((intptr_t)this + *pModelOffset);
                return internalModel;
            }
        }
        return NULL;
    }
};