#pragma once

#include "../../Graphics/Vector.h"
#include "../../World/BMDJ.h"

struct TileFeaturePlacementData
{
    // identifier for the tile type (e.g. D01A is 0, D02A is 1, ...)
    unsigned short tileID;
    // For each of the nine positions (NW, N, NE, ..., SE) this stores the
    // valid directions for a feature placed at said position. The upper four
    // bits are zero; bits 0, 1, 2, 3 correspond to whether it's possible to
    // place an object facing up, right, down, left respectively.
    unsigned char directionBitmasks[9];
    // Null-terminated string of length 4 encoding the type of tile.
    // For example, R04A for the fully open tile or E01A for the dead end.
    // The tile R02A (corresponding to a 270 degree corner in a room) has some
    // special behaviour, which I think is why it gets copied here.
    char tilename[5];
};

struct GrottoTileData
{
    Zone3D_BMDJStruct* bmdj;
    TileFeaturePlacementData featurePlacement;
    Matrix3x3 rotationMatrix;
    fix32_t rotationAngle;
    Vector3fix centrePosition;
};

enum TilePlacementPoint
{
    TilePlacementPoint_NW = 0,
    TilePlacementPoint_N = 1,
    TilePlacementPoint_NE = 2,
    TilePlacementPoint_W = 3,
    TilePlacementPoint_Center = 4,
    TilePlacementPoint_E = 5,
    TilePlacementPoint_SW = 6,
    TilePlacementPoint_S = 7,
    TilePlacementPoint_SE = 8,
};

int ChooseTileFeaturePosition(const TileFeaturePlacementData* data);
int ChooseTileFeatureOrientation(const TileFeaturePlacementData* data,
        int position, bool preferFaceDown);