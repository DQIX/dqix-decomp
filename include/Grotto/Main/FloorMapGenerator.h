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
    // The save editor calls this CreateDungeonDetail
    void Calculate(int floor, int unused);
    
private:
    // Randomly and recursively subdivides a partition rectangle
    void RoutineF(PartitionRect& part);

    // Splits a partition rectangle into two with a vertical cut
    bool RoutineE(PartitionRect& oldpart, PartitionRect& newpart, BoundaryRect& boundary);
    // Splits a partition rectangle into two with a horizontal cut
    bool RoutineA(PartitionRect& oldpart, PartitionRect& newpart, BoundaryRect& boundary);

    // Selects a subrectangle of the partition rectangle to use as a
    // base shape for the corresponding room. Returns true if a room was
    // successfully created (it can fail if the partition rectangle is small).
    bool RoutineC(PartitionRect& part, Room& room);

    // The save editor calls this GenerateFloorMap.
    // Reshapes a room by randomly expanding/eroding along each boundary point.
    void ReshapeRoom(const PartitionRect& part);

    // Connects the rooms on each side of the boundary via their join points,
    // creating a corridor shaped like a tetris S/Z-piece.
    bool RoutineG(const BoundaryRect& boundary);
    // Performs a single connection for RoutineG. If extend is true, the
    // 'middle' part of the S/Z-shape will be extended into a longer corridor.
    bool RoutineH(const GrottoTilePoint& p1, const GrottoTilePoint& p2,
        const BoundaryRect& boundary, bool extend);
    // Generates location of stairs
    bool RoutineJ();
    // Generates quantity and location of chests
    bool RoutineK();
    // Randomly assigns join points to a rectangular room
    void RoutineD(Room& room);

public:
    void Initialize();

private:
    int RandRange(int minimum, int maximum) const;
};