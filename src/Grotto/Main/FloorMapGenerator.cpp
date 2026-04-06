#include "Grotto/Main/FloorMapGenerator.h"
#include "std_library_functions.h"
#include <globaldefs.h>

#ifdef jpn
#define func_020c1180 func_020c2c4c
#define func_020c1280 func_020c2d4c
#endif

extern "C"
{
    void func_020c1180(void*);
    void func_020c1280(void*, int, int);
}

// I'm not going to replace tile values with an enum, but here's what
// they correspond to:
// 0 = generic walkable tile
// 1 = generic void tile
// 2 = corridor
// 3 = void tile forming part of a boundary
// 4 = stairs going up (there can be multiple of these, only one of them counts.
//                      rest are effectively generic walkable tiles)
// 5 = stairs going down
// 6 = chest
// 7 is unused
// 8 = join point of a room

// 1 and 3 correspond to blocked/void tiles, so this is replacing a tile value
// with a bool indicating whether it's blocked or not. Note that this is an
// idempotent operation, so when interpreting the effect of this you can ignore
// successive invocations on the same variable (at least for the purposes of
// function behaviour: they'll still affect the assembly output).
// also, don't try to write this with a ternary operator or implicit cast from
// bool to int... the compiler will freak out and use different registers
#define REDUCE_TILEVALUE(n) if (n == 1 || n == 3) n = 1; else n = 0

// USA: func_02090444
// JPN: func_02090d64
void FloorMapGenerator::Calculate(int floor)
{
    if (seed < 0 || pFloorMap == NULL)
        return;
    
    srand(seed + floor);
    int width = pFloorMap->width;
    int height = pFloorMap->height;

    partitionRects[0].bounds.left = 1;
    partitionRects[0].bounds.top = 1;
    partitionRects[0].bounds.right = width - 2;
    partitionRects[0].bounds.bottom = height - 2;

    RoutineF(partitionRects[0]);

    for (int i = 0; i < partitionRectCount; i++)
    {
        if (RoutineC(partitionRects[i], rooms[roomCount]) == true)
            roomCount++;
    }

    for (int i = 0; i < partitionRectCount; i++)
        ReshapeRoom(partitionRects[i]);

    for (int i = 0; i < boundaryCount; i++)
        RoutineG(boundaries[i]);

    for (int rx = 0; rx < pFloorMap->width; rx++)
    {
        pFloorMap->WriteTile(rx, 0, 1);
        pFloorMap->WriteTile(rx, pFloorMap->height - 1, 1);
    }

    for (int ry = 0; ry < pFloorMap->height; ry++)
    {
        pFloorMap->WriteTile(0, ry, 1);
        pFloorMap->WriteTile(pFloorMap->width - 1, ry, 1);
    }

    RoutineJ();
    if (floor > 2)
        RoutineK();
    else
        pFloorMap->chestCount = 0;
}

// USA: 0209060c
// JPN: 02090f2c
void FloorMapGenerator::RoutineF(PartitionRect& part)
{
    if (partitionRectCount >= 15)
        return;

    if (part.cutY)
    {
        if (!RoutineA(part, partitionRects[partitionRectCount], boundaries[boundaryCount]))
            return;
    }
    else if (part.cutX)
    {
        if (!RoutineE(part, partitionRects[partitionRectCount], boundaries[boundaryCount]))
            return;
    }
    else if (rand() % 2 != 0)
    {
        if (!RoutineE(part, partitionRects[partitionRectCount], boundaries[boundaryCount]))
            return;
    }
    else
    {
        if (!RoutineA(part, partitionRects[partitionRectCount], boundaries[boundaryCount]))
            return;
    }

    int nextIdx = partitionRectCount++;
    boundaryCount++;
    if (rand() % 2 != 0)
    {
        RoutineF(part);
        RoutineF(partitionRects[nextIdx]);
    }
    else
    {
        RoutineF(partitionRects[nextIdx]);
        RoutineF(part);
    }
}

// USA: func_0209076c
// JPN: func_0209108c
bool FloorMapGenerator::RoutineE(PartitionRect& oldpart, PartitionRect& newpart, BoundaryRect& boundary)
{
    if (oldpart.bounds.right - oldpart.bounds.left + 1 < 7)
        return false;

    if (oldpart.cutY)
        return false;

    // might need to reorder this sum
    int cutPos = oldpart.bounds.left + 3 +
        RandRange(0, oldpart.bounds.right - oldpart.bounds.left - 6);

    
    for (int ry = 0; ry < oldpart.bounds.GetHeight(); ry++)
    {
        pFloorMap->WriteTile(cutPos, oldpart.bounds.top + ry, 3);
    }

    newpart.bounds.left = cutPos + 1;
    newpart.bounds.top = oldpart.bounds.top;
    newpart.bounds.right = oldpart.bounds.right;
    newpart.bounds.bottom = oldpart.bounds.bottom;

    oldpart.bounds.right = cutPos - 1;
    oldpart.cutY = true;

    boundary.bounds.left = cutPos;
    boundary.bounds.top = oldpart.bounds.top;
    boundary.bounds.right = cutPos;
    boundary.bounds.bottom = oldpart.bounds.bottom;

    boundary.leftOrTop = &oldpart;
    boundary.rightOrBottom = &newpart;
    boundary.cutType = CutType_Vertical;
    
    return true;
}

// USA: func_02090860
// JPN: func_02091180
int FloorMapGenerator::BasicRectangle::GetHeight() const
{
    return bottom - top + 1;
}

// USA: func_02090874
// JPN: func_02091194
bool FloorMapGenerator::RoutineA(PartitionRect& oldpart, PartitionRect& newpart, BoundaryRect& boundary)
{
    if (oldpart.bounds.GetHeight() < 7)
        return false;

    if (oldpart.cutX)
        return false;

    int rngMax = oldpart.bounds.GetHeight() - 6;
    int cutPos = oldpart.bounds.top + 3 + RandRange(0, rngMax - 1);
    
    for (int rx = 0; rx < oldpart.bounds.right - oldpart.bounds.left + 1; rx++)
    {
        pFloorMap->WriteTile(oldpart.bounds.left + rx, cutPos, 3);
    }

    newpart.bounds.left = oldpart.bounds.left;
    newpart.bounds.top = cutPos + 1;
    newpart.bounds.right = oldpart.bounds.right;
    newpart.bounds.bottom = oldpart.bounds.bottom;

    oldpart.bounds.bottom = cutPos - 1;
    oldpart.cutX = true;

    boundary.bounds.left = oldpart.bounds.left;
    boundary.bounds.top = cutPos;
    boundary.bounds.right = oldpart.bounds.right;
    boundary.bounds.bottom = cutPos;

    boundary.leftOrTop = &oldpart;
    boundary.rightOrBottom = &newpart;
    boundary.cutType = CutType_Horizontal;
    
    return true;
}

// USA: func_0209096c
// JPN: func_0209128c
bool FloorMapGenerator::RoutineC(PartitionRect& part, Room& room)
{
    if (part.bounds.right - part.bounds.left + 1 < 3 ||
        part.bounds.GetHeight() < 3)
        return false;

    int roomLeft;
    int roomTop;
    int roomRight;
    int roomBottom;

    if (RandRange(0, 1) != 0)
    {
        roomLeft = RandRange(
            part.bounds.left,
            part.bounds.left + (part.bounds.right - part.bounds.left + 1) / 3);
        roomTop = RandRange(
            part.bounds.top,
            part.bounds.top + part.bounds.GetHeight() / 3);
    }
    else
    {
        roomLeft = RandRange(
            part.bounds.left + 1,
            part.bounds.left + (part.bounds.right - part.bounds.left + 1) / 3);
        roomTop = RandRange(
            part.bounds.top + 1,
            part.bounds.top + part.bounds.GetHeight() / 3);
    }

    if (RandRange(0, 1) != 0)
    {
        roomRight = RandRange(
            part.bounds.left + (part.bounds.right - part.bounds.left + 1) / 3 * 2,
            part.bounds.right);
        roomBottom = RandRange(
            part.bounds.top + part.bounds.GetHeight() / 3 * 2,
            part.bounds.bottom);
    }
    else
    {
        roomRight = RandRange(
            part.bounds.left + (part.bounds.right - part.bounds.left + 1) / 3 * 2,
            part.bounds.right - 1);
        roomBottom = RandRange(
            part.bounds.top + part.bounds.GetHeight() / 3 * 2,
            part.bounds.bottom - 1);
    }

    pFloorMap->WriteRectangle(roomLeft, roomTop, 
        roomRight - roomLeft + 1, roomBottom - roomTop + 1, 0);
    room.bounds.left = roomLeft;
    room.bounds.top = roomTop;
    room.bounds.right = roomRight;
    room.bounds.bottom = roomBottom;
    part.pRoom = &room;
    RoutineD(room);
    return true;
}

// USA: func_02090b8c
// JPN: func_020914ac
void FloorMapGenerator::ReshapeRoom(const PartitionRect& part)
{
    const Room& room = *(part.pRoom);
    room.bounds.GetHeight(); // wasted call?
    char halfWidth = (room.bounds.right - room.bounds.left + 1) / 2;
    char halfHeight = room.bounds.GetHeight() / 2;

    char topMargin;
    char bottomMargin;
    char leftMargin;
    char rightMargin;

    if (mainRoomAssigned || RandRange(0, 15) != 0)
    {
        topMargin = room.bounds.top - part.bounds.top;
        bottomMargin = part.bounds.bottom - room.bounds.bottom;
        leftMargin = room.bounds.left - part.bounds.left;
        rightMargin = part.bounds.right - room.bounds.right;
    }
    else
    {
        topMargin = room.bounds.top - 1;
        bottomMargin = pFloorMap->height - room.bounds.bottom - 1;
        leftMargin = room.bounds.left - 1;
        rightMargin = pFloorMap->width - room.bounds.right - 1;
        // needs reinterpretation. Pretty sure this room is still effectively
        // constrained to within its PartitionRect, because of the procedure
        // of putting 8s in all boundary tiles.
        mainRoomAssigned = true;
    }  

    for (int rx = room.bounds.left; rx <= room.bounds.right; rx++)
    {
        int topTile = pFloorMap->GetTile(rx, room.bounds.top);
        if (topTile == 1 || topTile == 3)
            continue;
        if (RandRange(0, 1) != 0)
        {
            unsigned char yexpand = RandRange(0, topMargin);
            for (int ry = 0; ry < yexpand; ry++)
            {
                if (pFloorMap->GetTile(rx, room.bounds.top - 1 - ry) != 8)
                    pFloorMap->WriteTile(rx, room.bounds.top - 1 - ry, 0);
            }
        }
        else
        {
            if (topTile == 8)
                continue;
            int outer = pFloorMap->GetTile(rx, room.bounds.top - 1);
            if (outer == 1 || outer == 8)
                continue;
            unsigned char yreduce = RandRange(0, halfHeight);
            for (int ry = 0; ry < yreduce; ry++)
            {
                if (pFloorMap->GetTile(rx, room.bounds.top + ry) == 8)
                    break;
                if (pFloorMap->GetTile(rx, room.bounds.top + ry + 1) == 1)
                    break;
                if (pFloorMap->GetTile(rx + 1, room.bounds.top + ry) != 1 &&
                    pFloorMap->GetTile(rx + 1, room.bounds.top + ry + 1) == 1)
                    break;
                if (pFloorMap->GetTile(rx - 1, room.bounds.top + ry) != 1 &&
                    pFloorMap->GetTile(rx - 1, room.bounds.top + ry + 1) == 1)
                    break;
                pFloorMap->WriteTile(rx, room.bounds.top + ry, 1);
            }
        }
    }

    for (int rx = room.bounds.left; rx <= room.bounds.right; rx++)
    {
        int bottomTile = pFloorMap->GetTile(rx, room.bounds.bottom);
        if (bottomTile == 1 || bottomTile == 3)
            continue;
        if (RandRange(0, 1) != 0)
        {
            unsigned char yexpand = RandRange(0, bottomMargin);
            for (int ry = 0; ry < yexpand; ry++)
            {
                if (pFloorMap->GetTile(rx, room.bounds.bottom + 1 + ry) != 8)
                    pFloorMap->WriteTile(rx, room.bounds.bottom + 1 + ry, 0);
            }
        }
        else
        {
            if (bottomTile == 8)
                continue;
            int outer = pFloorMap->GetTile(rx, room.bounds.bottom + 1);
            if (outer == 1 || outer == 8)
                continue;
            unsigned char yreduce = RandRange(0, halfHeight);
            for (int ry = 0; ry < yreduce; ry++)
            {
                if (pFloorMap->GetTile(rx, room.bounds.bottom - ry) == 8)
                    break;
                if (pFloorMap->GetTile(rx, room.bounds.bottom - ry - 1) == 1)
                    break;
                if (pFloorMap->GetTile(rx + 1, room.bounds.bottom - ry) != 1 &&
                    pFloorMap->GetTile(rx + 1, room.bounds.bottom - ry - 1) == 1)
                    break;
                if (pFloorMap->GetTile(rx - 1, room.bounds.bottom - ry) != 1 &&
                    pFloorMap->GetTile(rx - 1, room.bounds.bottom - ry - 1) == 1)
                    break;
                pFloorMap->WriteTile(rx, room.bounds.bottom - ry, 1);
            }
        }
    }

    for (int ry = room.bounds.top; ry <= room.bounds.bottom; ry++)
    {
        int edgeTile = pFloorMap->GetTile(room.bounds.left, ry);
        if (edgeTile == 1 || edgeTile == 3)
            continue;
        if (RandRange(0, 1) != 0)
        {
            unsigned char xexpand = RandRange(0, leftMargin);
            for (int rx = 0; rx < xexpand; rx++)
            {
                if (pFloorMap->GetTile(room.bounds.left - 1 - rx, ry) != 8)
                    pFloorMap->WriteTile(room.bounds.left - 1 - rx, ry, 0);
            }
        }
        else
        {
            if (edgeTile == 8)
                continue;
            int outer = pFloorMap->GetTile(room.bounds.left - 1, ry);
            if (outer == 1 || outer == 8)
                continue;
            unsigned char xreduce = RandRange(0, halfWidth);
            for (int rx = 0; rx < xreduce; rx++)
            {
                if (pFloorMap->GetTile(room.bounds.left + rx, ry) == 8)
                    break;
                if (pFloorMap->GetTile(room.bounds.left + rx + 1, ry) == 1)
                    break;
                if (pFloorMap->GetTile(room.bounds.left + rx, ry + 1) != 1 &&
                    pFloorMap->GetTile(room.bounds.left + rx + 1, ry + 1) == 1)
                    break;
                if (pFloorMap->GetTile(room.bounds.left + rx, ry - 1) != 1 &&
                    pFloorMap->GetTile(room.bounds.left + rx + 1, ry - 1) == 1)
                    break;
                pFloorMap->WriteTile(room.bounds.left + rx, ry, 1);
            }
        }
    }

    for (int ry = room.bounds.top; ry <= room.bounds.bottom; ry++)
    {
        int edgeTile = pFloorMap->GetTile(room.bounds.right, ry);
        if (edgeTile == 1 || edgeTile == 3)
            continue;
        if (RandRange(0, 1) != 0)
        {
            unsigned char xexpand = RandRange(0, rightMargin);
            for (int rx = 0; rx < xexpand; rx++)
            {
                if (pFloorMap->GetTile(room.bounds.right + 1 + rx, ry) != 8)
                    pFloorMap->WriteTile(room.bounds.right + 1 + rx, ry, 0);
            }
        }
        else
        {
            if (edgeTile == 8)
                continue;
            // bug? Would make much more sense as bounds.right + 1
            int outer = pFloorMap->GetTile(room.bounds.top + 1, ry);
            if (outer == 1 || outer == 8)
                continue;
            unsigned char xreduce = RandRange(0, halfWidth);
            for (int rx = 0; rx < xreduce; rx++)
            {
                if (pFloorMap->GetTile(room.bounds.right - rx, ry) == 8)
                    break;
                if (pFloorMap->GetTile(room.bounds.right - rx - 1, ry) == 1)
                    break;
                if (pFloorMap->GetTile(room.bounds.right - rx, ry + 1) != 1 &&
                    pFloorMap->GetTile(room.bounds.right - rx - 1, ry + 1) == 1)
                    break;
                if (pFloorMap->GetTile(room.bounds.right - rx, ry - 1) != 1 &&
                    pFloorMap->GetTile(room.bounds.right - rx - 1, ry - 1) == 1)
                    break;
                pFloorMap->WriteTile(room.bounds.right - rx, ry, 1);
            }
        }
    }
}

// USA: func_02091448
// JPN: func_02091d68
bool FloorMapGenerator::RoutineG(const BoundaryRect& boundary)
{
    if (boundary.cutType == CutType_Horizontal)
    {
        const Room* top = boundary.leftOrTop->pRoom;
        const Room* bottom = boundary.rightOrBottom->pRoom;
        bool extend = RandRange(0, 7) == 0;
        RoutineH(top->joinPoints[6], bottom->joinPoints[4], boundary, extend);
        extend = RandRange(0, 7) == 0;
        RoutineH(top->joinPoints[7], bottom->joinPoints[4], boundary, extend);
        extend = RandRange(0, 7) == 0;
        RoutineH(top->joinPoints[6], bottom->joinPoints[5], boundary, extend);
        extend = RandRange(0, 7) == 0;
        RoutineH(top->joinPoints[7], bottom->joinPoints[5], boundary, extend);
    }
    else if (boundary.cutType == CutType_Vertical)
    {
        const Room* left = boundary.leftOrTop->pRoom;
        const Room* right = boundary.rightOrBottom->pRoom;
        bool extend = RandRange(0, 7) == 0;
        RoutineH(left->joinPoints[2], right->joinPoints[0], boundary, extend);
        extend = RandRange(0, 7) == 0;
        RoutineH(left->joinPoints[3], right->joinPoints[0], boundary, extend);
        extend = RandRange(0, 7) == 0;
        RoutineH(left->joinPoints[2], right->joinPoints[1], boundary, extend);
        extend = RandRange(0, 7) == 0;
        RoutineH(left->joinPoints[3], right->joinPoints[1], boundary, extend);
    }

    return true;
}

// USA: func_0209162c
// JPN: func_02091f4c
bool FloorMapGenerator::RoutineH(const GrottoTilePoint& p1, const GrottoTilePoint& p2,
    const BoundaryRect& boundary, bool extend)
{
    if (p1.x == 0 || p1.y == 0 || p2.x == 0 || p2.y == 0)
        return false;

    if (boundary.cutType == CutType_Horizontal)
    {
        for (int ry = p1.y + 1; ry < boundary.bounds.top + 1; ry++)
            pFloorMap->WriteTile(p1.x, ry, 2);

        for (int ry = p2.y - 1; ry > boundary.bounds.top; ry--)
            pFloorMap->WriteTile(p2.x, ry, 2);

        if (p1.x < p2.x)
        {
            for (int rx = p1.x; rx < p2.x + 1; rx++)
                pFloorMap->WriteTile(rx, boundary.bounds.top, 2);
        }
        else if (p1.x > p2.x)
        {
            for (int rx = p2.x; rx < p1.x + 1; rx++)
                pFloorMap->WriteTile(rx, boundary.bounds.top, 2);
        }

        if (extend)
        {
            if (p1.x < p2.x)
            {
                for (int limit = p2.x + 1; limit < 16; limit++)
                {
                    int tileValue = pFloorMap->GetTile(limit, boundary.bounds.top);
                    if (tileValue == 1 || tileValue == 3)
                        continue;
                    for (int rx = p2.x + 1; rx < limit; rx++)
                        pFloorMap->WriteTile(rx, boundary.bounds.top, 2);
                    break;
                }
                for (int limit = p1.x - 1; limit >= 0; limit--)
                {
                    int tileValue = pFloorMap->GetTile(limit, boundary.bounds.top);
                    if (tileValue == 1 || tileValue == 3)
                        continue;
                    for (int rx = p1.x - 1; rx > limit; rx--)
                        pFloorMap->WriteTile(rx, boundary.bounds.top, 2);
                    break;
                }
            }
            else if (p1.x >= p2.x)
            {
                for (int limit = p1.x + 1; limit < 16; limit++)
                {
                    int tileValue = pFloorMap->GetTile(limit, boundary.bounds.top);
                    if (tileValue == 1 || tileValue == 3)
                        continue;
                    for (int rx = p1.x + 1; rx < limit; rx++)
                        pFloorMap->WriteTile(rx, boundary.bounds.top, 2);
                    break;
                }
                for (int limit = p2.x - 1; limit >= 0; limit--)
                {
                    int tileValue = pFloorMap->GetTile(limit, boundary.bounds.top);
                    if (tileValue == 1 || tileValue == 3)
                        continue;
                    for (int rx = p2.x - 1; rx > limit; rx--)
                        pFloorMap->WriteTile(rx, boundary.bounds.top, 2);
                    break;
                }
            }
        }
    }
    else if (boundary.cutType == CutType_Vertical)
    {
        for (int rx = p1.x + 1; rx < boundary.bounds.left + 1; rx++)
            pFloorMap->WriteTile(rx, p1.y, 2);
        for (int rx = p2.x - 1; rx > boundary.bounds.left; rx--)
            pFloorMap->WriteTile(rx, p2.y, 2);

        if (p1.y < p2.y)
        {
            for (int ry = p1.y; ry < p2.y + 1; ry++)
                pFloorMap->WriteTile(boundary.bounds.left, ry, 2);
        }
        else if (p1.y > p2.y)
        {
            for (int ry = p2.y; ry < p1.y + 1; ry++)
                pFloorMap->WriteTile(boundary.bounds.left, ry, 2);
        }

        if (extend)
        {
            if (p1.y < p2.y)
            {
                for (int limit = p2.y + 1; limit < 16; limit++)
                {
                    int tileValue = pFloorMap->GetTile(boundary.bounds.left, limit);
                    if (tileValue == 1 || tileValue == 3)
                        continue;
                    for (int ry = p2.y + 1; ry < limit; ry++)
                        pFloorMap->WriteTile(boundary.bounds.left, ry, 2);
                    break;
                }

                for (int limit = p1.y - 1; limit >= 0; limit--)
                {
                    int tileValue = pFloorMap->GetTile(boundary.bounds.left, limit);
                    if (tileValue == 1 || tileValue == 3)
                        continue;
                    for (int ry = p1.y - 1; ry > limit; ry--)
                        pFloorMap->WriteTile(boundary.bounds.left, ry, 2);
                    break;
                }
            }
            else if (p1.y >= p2.y)
            {
                for (int limit = p1.y + 1; limit < 16; limit++)
                {
                    int tileValue = pFloorMap->GetTile(boundary.bounds.left, limit);
                    if (tileValue == 1 || tileValue == 3)
                        continue;
                    for (int ry = p1.y + 1; ry < limit; ry++)
                        pFloorMap->WriteTile(boundary.bounds.left, ry, 2);
                    break;
                }

                for (int limit = p2.y - 1; limit >= 0; limit--)
                {
                    int tileValue = pFloorMap->GetTile(boundary.bounds.left, limit);
                    if (tileValue == 1 || tileValue == 3)
                        continue;
                    for (int ry = p2.y - 1; ry > limit; ry--)
                        pFloorMap->WriteTile(boundary.bounds.left, ry, 2);
                    break;
                }
            }
        }
    }
    
    return true;
}

// USA: func_02091bc0
// JPN: func_020924e0
bool FloorMapGenerator::RoutineJ()
{    
    int upRoomIdx;
    Room room;
    
    int numUpAttempts = 0;
    int numDownAttempts = 0;

    int posX, posY;
    while (true)
    {
        upRoomIdx = RandRange(0, roomCount - 1);
        room.CopyFrom(rooms[upRoomIdx]);
        posX = RandRange(room.bounds.left, room.bounds.right);
        posY = RandRange(room.bounds.top, room.bounds.bottom);
        int tileN, tileS, tileW, tileE;
        if (numUpAttempts < 100)
        {
            tileN = pFloorMap->GetTile(posX, posY - 1);
            tileS = pFloorMap->GetTile(posX, posY + 1);
            tileW = pFloorMap->GetTile(posX - 1, posY);
            tileE = pFloorMap->GetTile(posX + 1, posY);
            pFloorMap->GetTile(posX + 1, posY - 1);
            pFloorMap->GetTile(posX + 1, posY + 1);
            pFloorMap->GetTile(posX - 1, posY + 1);
            pFloorMap->GetTile(posX - 1, posY - 1);
        
            REDUCE_TILEVALUE(tileN);
            REDUCE_TILEVALUE(tileS);
            REDUCE_TILEVALUE(tileW);
            REDUCE_TILEVALUE(tileE);
        
            if (tileN && tileS && tileW && tileE)
            {
                numUpAttempts++;
                continue;
            }
        
            if ((tileN && tileE) || (tileN && tileW) ||
                (tileS && tileE) || (tileS && tileW))
            {
                numUpAttempts++;
                continue;
            }
            
            unsigned char adjacency;
            pFloorMap->ComputeSingleAdjacencyDataPoint(posX, posY, &adjacency);
            if (adjacency == 0xEA || adjacency == 0xAB || adjacency == 0xAE || adjacency == 0xBA ||
                 adjacency == 0xA3 || adjacency == 0x8E || adjacency == 0x3A || adjacency == 0xE8 ||
                 adjacency == 0xB8 || adjacency == 0xE2 || adjacency == 0x8B || adjacency == 0x2E)
            {
                numUpAttempts++;
                continue;
            }
        }
        
        {
            FloorMap* map = pFloorMap;
            if (tileS != 1 && tileS != 3)
            {
                map->fpUpStairRotation = 0;
                func_020c1180(map->unknown1);
            }
            else if (tileE != 1 && tileE != 3)
            {
                map->fpUpStairRotation = 0x1921;
                func_020c1280(map->unknown1, 0x1000, 0);
            }
            else if (tileW != 1 && tileW != 3)
            {
                map->fpUpStairRotation = 0x3243;
                func_020c1280(map->unknown1, 0, -0x1000);
            }
            else if (tileN != 1 && tileN != 3)
            {
                map->fpUpStairRotation = 0x4b65;
                func_020c1280(map->unknown1, -0x1000, 0);
            }
            map->upStairWorldX = (int)((float)posX * 8.0f * 4096.0f);
            map->upStairWorldY = 0;
            map->upStairWorldZ = (int)((float)posY * 8.0f * 4096.0f);
        }
        pFloorMap->WriteTile(posX, posY, 4);
        {
            FloorMap* map = this->pFloorMap;
            map->stairsUp.x = posX;
            map->stairsUp.y = posY;
        }
    
        int oldUpX, oldUpY;
        oldUpX = posX;
        oldUpY = posY;
        numUpAttempts = 0;
        Room* roomArray = this->rooms;
        
        while (true)
        {
            int downRoomIdx = RandRange(0, roomCount - 1);
            if (upRoomIdx == downRoomIdx && numDownAttempts < 25)
            {
                numDownAttempts++;
                continue;
            }
            
            room.CopyFrom(roomArray[downRoomIdx]);
            posX = RandRange(room.bounds.left, room.bounds.right);
            posY = RandRange(room.bounds.top, room.bounds.bottom);
            if (downRoomIdx == upRoomIdx && posX == oldUpX && posY == oldUpY)
                continue;
            break;
        }
    
        tileN = pFloorMap->GetTile(posX, posY - 1);
        tileS = pFloorMap->GetTile(posX, posY + 1);
        tileW = pFloorMap->GetTile(posX - 1, posY);
        tileE = pFloorMap->GetTile(posX + 1, posY);
        
        REDUCE_TILEVALUE(tileN);
        REDUCE_TILEVALUE(tileS);
        REDUCE_TILEVALUE(tileW);
        REDUCE_TILEVALUE(tileE);
    
        if (tileN && tileS && tileW && tileE)
        {
            numUpAttempts++;
            continue;
        }
    
        if ((tileN && tileE) || (tileN && tileW) ||
            (tileS && tileE) || (tileS && tileW))
        {
            numUpAttempts++;
            continue;
        }
        break;
    }
    
    {
        FloorMap* map = pFloorMap;
        map->fpDownStairRotation = 0;
        func_020c1180(map->unknown2);
        map->downStairWorldX = (int)((float)posX * 8.0f * 4096.0f);
        map->downStairWorldY = 0;
        map->downStairWorldZ = (int)((float)posY * 8.0f * 4096.0f);
    }
    pFloorMap->WriteTile(posX, posY, 5);
    {
        FloorMap* map = pFloorMap;
        map->stairsDown.x = posX;
        map->stairsDown.y = posY;
    }
    return true;
}

// USA: func_020920d0
// JPN: func_020929f0
ARM FloorMapGenerator::Room& FloorMapGenerator::Room::CopyFrom(const Room& other)
{
    // Pretty sure these were all calls to memcpy that got inlined. If the exact
    // same function signature works in other places, it might be worth putting
    // in the std library header.
#define INLINE_MEMCPY(to, from, size) \
    { \
        int i; \
        unsigned char* dst; \
        const unsigned char* src; \
        i = (size); \
        src = (const unsigned char*)(from); \
        dst = (unsigned char*)(to); \
        do { \
            i--; \
            *dst = *src; \
            dst++; \
            src++; \
        } while (i != 0); \
    }

    INLINE_MEMCPY(&bounds, &other.bounds, 4);
    INLINE_MEMCPY(&joinPoints[0], &other.joinPoints[0], 4);
    INLINE_MEMCPY(&joinPoints[2], &other.joinPoints[2], 4);
    INLINE_MEMCPY(&joinPoints[4], &other.joinPoints[4], 4);
    INLINE_MEMCPY(&joinPoints[6], &other.joinPoints[6], 4);

    return *this;
}

// USA: func_02092164
// JPN: func_02092a84
ARM bool FloorMapGenerator::RoutineK()
{
    // This function technically has UB and reads these values uninitialized
    // from registers r4, r5, r6, r7 respectively. It looks like they may have
    // originally been designed to test for neighbouring tiles. In practice,
    // when this function is called,
    // - r4 holds the current floor
    // - r5 is the this pointer
    // - r6 = 1 and r7 = 0.
    // In particular, on the third floor we will fail 100 attempts, which has
    // the effect of advancing AT by 300 before generating each chest.
    int u4, u5, u6, u7;
    
    int numChests = RandRange(1, 3);
    
    pFloorMap->chestCount = numChests;
    int numAttempts = 0;
    
    for (int chestIdx = 0; chestIdx < numChests; chestIdx++)
    {
        while (true)
        {
            Room room = rooms[RandRange(0, roomCount - 1)];       
            
            unsigned int locX = RandRange(room.bounds.left, room.bounds.right);
            unsigned int locY = RandRange(room.bounds.top, room.bounds.bottom);
            int currentTileValue = pFloorMap->GetTile(locX, locY);
    
            if (numAttempts < 100)
            {
                REDUCE_TILEVALUE(u4);
                REDUCE_TILEVALUE(u5);
                REDUCE_TILEVALUE(u6);
                REDUCE_TILEVALUE(u7);
                
                if (u4 && u5 && u6 && u7)
                {
                    numAttempts++;
                    continue;
                }
    
                if ((u4 && u7) || (u4 && u6) ||
                   (u5 && u7) || (u5 && u6))
                {
                    numAttempts++;
                    continue;
                }
            }

            if (currentTileValue == 6)
                continue;
    
            if ((unsigned int)(currentTileValue - 4) <= 1)
            {
                numAttempts++;
                continue;
            }
    
            pFloorMap->WriteTile(locX, locY, 6);
            unsigned char chestIdxU8 = chestIdx;
            FloorMap* evil = (FloorMap*)((char*)(pFloorMap) + 2 * chestIdxU8);
            evil->chests->x = locX;
            evil->chests->y = locY;
            break;
        }
    }

    return true;
}

// USA: func_020922fc
// JPN: func_02092c1c
ARM void FloorMapGenerator::RoutineD(Room& room)
{
    if (room.bounds.left == 0 || room.bounds.top == 0 ||
        room.bounds.right == 0 || room.bounds.bottom == 0)
        return;

    if (room.bounds.right - room.bounds.left + 1 < 5)
    {
        char rng = RandRange(room.bounds.left, room.bounds.right);
        room.joinPoints[4].x = rng;
        room.joinPoints[4].y = room.bounds.top;

        rng = RandRange(room.bounds.left, room.bounds.right);
        room.joinPoints[6].x = rng;
        room.joinPoints[6].y = room.bounds.bottom;
        
        pFloorMap->WriteTile(room.joinPoints[4].x, room.joinPoints[4].y, 8);
        pFloorMap->WriteTile(room.joinPoints[6].x, room.joinPoints[6].y, 8);
    }
    else
    {
        char middle = room.bounds.left + (room.bounds.right - room.bounds.left + 1) / 2 - 1;

        char rng = RandRange(room.bounds.left, middle);
        room.joinPoints[4].x = rng;
        room.joinPoints[4].y = room.bounds.top;

        rng = RandRange(middle + 1, room.bounds.right);
        room.joinPoints[5].x = rng;
        room.joinPoints[5].y = room.bounds.top;

        rng = RandRange(room.bounds.left, middle);
        room.joinPoints[6].x = rng;
        room.joinPoints[6].y = room.bounds.bottom;

        rng = RandRange(middle + 1, room.bounds.right);
        room.joinPoints[7].x = rng;
        room.joinPoints[7].y = room.bounds.bottom;

        pFloorMap->WriteTile(room.joinPoints[4].x, room.joinPoints[4].y, 8);
        pFloorMap->WriteTile(room.joinPoints[5].x, room.joinPoints[5].y, 8);
        pFloorMap->WriteTile(room.joinPoints[6].x, room.joinPoints[6].y, 8);
        pFloorMap->WriteTile(room.joinPoints[7].x, room.joinPoints[7].y, 8);
    }

    if (room.bounds.GetHeight() < 5)
    {
        char rng = RandRange(room.bounds.top, room.bounds.bottom);
        room.joinPoints[0].x = room.bounds.left;
        room.joinPoints[0].y = rng;
        
        rng = RandRange(room.bounds.top, room.bounds.bottom);
        room.joinPoints[2].x = room.bounds.right;
        room.joinPoints[2].y = rng;
        
        pFloorMap->WriteTile(room.joinPoints[0].x, room.joinPoints[0].y, 8);
        pFloorMap->WriteTile(room.joinPoints[2].x, room.joinPoints[2].y, 8);
    }
    else
    {
        char middle = room.bounds.top + room.bounds.GetHeight() / 2 - 1;

        char rng = RandRange(room.bounds.top, middle);
        room.joinPoints[0].x = room.bounds.left;
        room.joinPoints[0].y = rng;

        rng = RandRange(middle + 1, room.bounds.bottom);
        room.joinPoints[1].x = room.bounds.left;
        room.joinPoints[1].y = rng;

        rng = RandRange(room.bounds.top, middle);
        room.joinPoints[2].x = room.bounds.right;
        room.joinPoints[2].y = rng;

        rng = RandRange(middle + 1, room.bounds.bottom);
        room.joinPoints[3].x = room.bounds.right;
        room.joinPoints[3].y = rng;

        pFloorMap->WriteTile(room.joinPoints[0].x, room.joinPoints[0].y, 8);
        pFloorMap->WriteTile(room.joinPoints[1].x, room.joinPoints[1].y, 8);
        pFloorMap->WriteTile(room.joinPoints[2].x, room.joinPoints[2].y, 8);
        pFloorMap->WriteTile(room.joinPoints[3].x, room.joinPoints[3].y, 8);
    }
}

// USA: func_020925b8
// JPN: func_02092ed8
ARM void FloorMapGenerator::Initialize()
{
    seed = -1;
    pFloorMap = NULL;
    partitionRectCount = 1;
    boundaryCount = 0;
    roomCount = 0;
    mainRoomAssigned = false;

    memset(partitionRects, 0, sizeof(partitionRects));
    memset(boundaries, 0, sizeof(boundaries));
    memset(rooms, 0, sizeof(rooms));
}

// USA: func_02092614
// JPN: func_02092f34
ARM int FloorMapGenerator::RandRange(int minimum, int maximum) const
{
    if (minimum == maximum)
        return minimum;
    
    return minimum + (rand() % (maximum - minimum + 1));
}

