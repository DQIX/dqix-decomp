#pragma once

#include "FloorMap.h"

class FloorMapGenerator
{
public:
    struct BasicRectangle
    {
        char left;
        char top;
        char right;
        char bottom;

        // Weirdly there doesn't seem to be an analogous function for width.
        int GetHeight() const;
    };

    struct Room;

    struct PartitionRect
    {
        BasicRectangle bounds;
        bool cutX;
        bool cutY;
        char padding[2];
        Room* pRoom;
    };

    enum eCutType
    {
        CutType_Horizontal = 1,
        CutType_Vertical = 2
    };

    struct BoundaryRect
    {
        BasicRectangle bounds;
        PartitionRect* leftOrTop;
        PartitionRect* rightOrBottom;
        eCutType cutType;
    };

    struct Room
    {
        BasicRectangle bounds;
        GrottoTilePoint joinPoints[8];

        // In the binary this is placed right after RoutineJ, so it might
        // just be a static function.
        Room& CopyFrom(const Room& other);
    };

    FloorMap* pFloorMap;
    int seed;
    int partitionRectCount;
    int boundaryCount;
    int roomCount;

    PartitionRect partitionRects[16];
    BoundaryRect boundaries[16];
    Room rooms[16];

    bool mainRoomAssigned;

public:
    bool RoutineG(const BoundaryRect& boundary);
    bool RoutineH(const GrottoTilePoint& p1, const GrottoTilePoint& p2,
        const BoundaryRect& boundary, bool extend);
    bool RoutineJ();

    bool RoutineK();
    void RoutineD(Room& room);

    void Initialize();
    int RandRange(int minimum, int maximum) const;
};