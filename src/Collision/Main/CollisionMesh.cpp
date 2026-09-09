#include <globaldefs.h>
#include "std_library_functions.h"
#include "Collision/Main/CollisionMesh.h"

// USA: func_0204bf30
ARM void ClearCollisionMesh(struct CollisionMesh* mesh)
{
    memset(mesh, 0, sizeof(struct CollisionMesh));
}

// USA: func_0204bf44
//
// Resolves the file's five offsets against its own base, scales the bounding
// box up by the header's shift, and derives the cell count. Nothing is copied -
// the mesh points into the loaded image.
ARM void InitCollisionMesh(struct CollisionMesh* mesh, void* unused, const void* data)
{
    mesh->header = (const struct CollisionHeader*)data;

    mesh->triangles = (const char*)data + mesh->header->triangleOffset;
    mesh->cellCounts = (const unsigned char*)((const char*)data + mesh->header->cellCountOffset);
    mesh->cellStarts = (const unsigned short*)((const char*)data + mesh->header->cellStartOffset);
    mesh->triangleIndices =
        (const unsigned short*)((const char*)data + mesh->header->triangleIndexOffset);

    if (mesh->header->version == 3)
        mesh->trailing = (const char*)data + mesh->header->trailingOffset;

    mesh->boundsMin[0] = mesh->header->boundsMin[0] << mesh->header->boundsShift;
    mesh->boundsMin[1] = mesh->header->boundsMin[1] << mesh->header->boundsShift;
    mesh->boundsMin[2] = mesh->header->boundsMin[2] << mesh->header->boundsShift;

    mesh->boundsMax[0] = mesh->header->boundsMax[0] << mesh->header->boundsShift;
    mesh->boundsMax[1] = mesh->header->boundsMax[1] << mesh->header->boundsShift;
    mesh->boundsMax[2] = mesh->header->boundsMax[2] << mesh->header->boundsShift;

    mesh->cellCount = mesh->header->gridX * mesh->header->gridZ + mesh->header->gridZ / 2;
}
