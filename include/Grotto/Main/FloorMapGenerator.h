#pragma once

struct GrottoTilePoint
{
    char x;
    char y;
};

class FloorMap
{
public:
    // This is held externally, though seems to be held right after the 
    // end of the generator.
    char* pMapData; 
    char* unknown; // Looks like it points to end of map data.

    int width;
    int height;
    GrottoTilePoint stairsUp;
    GrottoTilePoint stairsDown;

    // More to come...
};

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

    void Initialize();
    int RandRange(int minimum, int maximum) const;
};