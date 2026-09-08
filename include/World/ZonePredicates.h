#pragma once

// Zone-ID range checks used to classify a zone before its map archive is
// loaded. All three are pure range tests over the same block of IDs.
//
// The grotto block spans 0x9C41..0xA221 (40001..41505 decimal). IsGrottoZone
// covers the whole block; the other two cover disjoint sub-ranges of it:
//
//     0x9C41 .. 0x9CA0   IsMainGrottoFloorZone   (96 IDs)
//     0x9CA1 .. 0xA08C   neither                 (1004 IDs)
//     0xA08D .. 0xA221   IsGrottoBossFloorZone   (405 IDs)
//
// Zone3D::LoadMapAMBL and Zone3D::LoadMapAMDJ branch on the two sub-ranges to
// pick which archive name to build, so they select a map's file naming:
//
//     main floor -> "Z0%dM01.ambl", environ clamped to 1..5
//     boss floor -> "Z0%dM99.ambl", environ unclamped
//     neither    -> "%s.ambl", named by the map code from maplist9.bin
//
// M01/M99 are fixed templates rather than authored maps, which is consistent
// with TreasureMapMetadata distinguishing a legacy boss map.

extern "C"
{

// usa: func_0201b588
bool IsGrottoZone(int zoneID);

// usa: func_0201b5b0
bool IsMainGrottoFloorZone(int zoneID);

// usa: func_0201b5d8
bool IsGrottoBossFloorZone(int zoneID);

}
