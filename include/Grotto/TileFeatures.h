#pragma once

struct TileFeaturePlacementData
{
    unsigned char unk[2];
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

int ChooseTileFeaturePosition(const TileFeaturePlacementData* data);
int ChooseTileFeatureOrientation(const TileFeaturePlacementData* data,
        int position, bool preferFaceDown);