#pragma once

#include "Common.h"

struct Nitro3DBoundingBox
{
    // all quantities are 16-bit fixed points.
    // scurest nsbmd docs say last 3 are xMax etc but that's not how
    // they're used in functions here
    fix16_t xMin;
    fix16_t yMin;
    fix16_t zMin;
    fix16_t xSize;
    fix16_t ySize;
    fix16_t zSize;
};

struct Nitro3DMesh
{
    // determines a function pointer to call in render command 5 (draw mesh)
    // in practice only 0 is supported here
    uint16_t drawMethod;
    uint16_t sizeBytes;
    // bit 1 used in LightingManager, no idea about rest
    uint32_t flags;
    uint32_t gpuCommandsOffset;
    uint32_t gpuCommandsLength; // length in bytes

    inline const uint32_t* GetGPUCommands() const
    {
        return (uint32_t*)((intptr_t)this + gpuCommandsOffset);
    }
};

struct Nitro3DInvBindMatrix
{
    Matrix4x3 mat4x3;
    Matrix3x3 mat3x3;
};

struct Nitro3DModel
{
    uint32_t fileSize;
    uint32_t renderCommandsOffset;
    uint32_t materialsOffset;
    uint32_t meshesOffset; // offset to a NameList of NSBXXMesh
    uint32_t inverseBindsOffset;
    char unk_14[1];
    // 0, 1 or 2, determines the behavior of applying scaling
    // to bones. 0 = simplest, only have one scaling operation followed
    // by rotation and translation.
    // In modes 1 and 2, each bone also provides a scale factor to be
    // used by its children.
    // Might have something to do with 'segment scale compensate'
    // behaviour? https://download.autodesk.com/us/maya/2011help/nodes/joint.html
    uint8_t boneScalingMode;
    uint8_t materialCallbackType;
    uint8_t numBoneMatrices;
    uint8_t numMaterials;
    uint8_t numMeshes;
    char unk_1a[2];
    fix32_t upScale; 
    fix32_t downScale; 
    uint16_t numVertices;
    uint16_t numPolygons;
    uint16_t numTriangles;
    uint16_t numQuads;
    Nitro3DBoundingBox bounds;
    fix32_t scale;
    char unk_3c[4];
    Nitro3DNameList boneList;

    // using this seems to screw up register assignment, but it's here in case
    // we can make it work later somehow
    inline Nitro3DModelMaterials* GetMaterialData() const
    {
        if (this != NULL && materialsOffset != 0)
            return (Nitro3DModelMaterials*)((intptr_t)this + materialsOffset);
        else
            return NULL;
    }

    inline Nitro3DNameList* GetMeshListUnsafe() const
    {
        return (Nitro3DNameList*)((intptr_t)this + meshesOffset);
    }

    inline Nitro3DNameList* GetMeshListSafe() const
    {
        return (this != NULL && meshesOffset != 0) ? GetMeshListUnsafe() : NULL;
    }

    inline Nitro3DNameList* GetMeshList() const
    {
        if (this != NULL && meshesOffset != 0)
            return (Nitro3DNameList*)((intptr_t)this + meshesOffset);
        return NULL;
    }
};

// represents an MDL0 subfile in an NSBMD
struct Nitro3DModelSet
{
    uint32_t signature;
    char unk_4[4];
    Nitro3DNameList nameList;

    Nitro3DModel* GetModelByIndex(unsigned int n) const
    {
        Nitro3DModel* internalModel;
        if (this != NULL)
        {
            uint32_t* pModelOffset = nameList.GetEntryByIndex<uint32_t>(n);

            if (pModelOffset != NULL)
            {
                internalModel = (Nitro3DModel*)((intptr_t)this + *pModelOffset);
                return internalModel;
            }
        }
        return NULL;
    }
};