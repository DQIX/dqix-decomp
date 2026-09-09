#pragma once

// The .col2 map collision mesh, as the game reads it.
//
// The file layout below was established by parsing the retail files directly,
// independently of this disassembly; the parser here confirms it field for
// field, and settles two things that file parsing alone could not.
struct CollisionHeader
{
    // 3 on every file that ships. The parser tests it, and only reads the
    // trailing section when it holds - so it is a version, not a constant.
    /* 0x00 */ unsigned int version;
    // A left shift, 0..5, applied to each bounding box component to get world
    // units. The box is stored scaled down by this many bits.
    /* 0x04 */ unsigned int boundsShift;
    /* 0x08 */ short boundsMin[3];
    /* 0x0e */ short boundsMax[3];
    /* 0x14 */ unsigned int triangleCount;
    /* 0x18 */ unsigned short cellSize;
    /* 0x1a */ unsigned short unknown_0x1a;
    /* 0x1c */ unsigned int gridX;
    /* 0x20 */ unsigned int gridZ;
    // All five are byte offsets from the start of the file.
    /* 0x24 */ unsigned int triangleOffset;
    /* 0x28 */ unsigned int cellCountOffset;
    /* 0x2c */ unsigned int cellStartOffset;
    /* 0x30 */ unsigned int triangleIndexOffset;
    /* 0x34 */ unsigned int trailingCount;
    /* 0x38 */ unsigned int trailingOffset;
};

// What the loader builds over a .col2 image: the offsets resolved to pointers,
// the bounding box scaled up, and the cell count derived. 0x4c bytes.
struct CollisionMesh
{
    /* 0x00 */ const CollisionHeader* header;
    /* 0x04 */ const void* triangles;
    /* 0x08 */ const unsigned char* cellCounts;
    /* 0x0c */ const unsigned short* cellStarts;
    /* 0x10 */ const unsigned short* triangleIndices;
    // Left null unless the header's version is 3.
    /* 0x14 */ const void* trailing;
    /* 0x18 */ int boundsMax[3];
    /* 0x24 */ int boundsMin[3];
    /* 0x30 */ char unknown_0x30[0x48 - 0x30];
    // gridX * gridZ + gridZ / 2.
    /* 0x48 */ int cellCount;
};

extern "C"
{

// USA: func_0204bf30
void ClearCollisionMesh(struct CollisionMesh* mesh);

// USA: func_0204bf44
void InitCollisionMesh(struct CollisionMesh* mesh, void* unused, const void* data);

}
