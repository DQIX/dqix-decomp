#include "Grotto/Main/FloorMapGenerator.h"
#include "std_library_functions.h"
#include <globaldefs.h>

// USA: func_02092164
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
    // the effect of advancing AT by 300.
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
                // Do not condense this into a ternary operator or bool -> int
                // conversion, the compiler will put these in r6-r9 instead
                if (u4 == 1 || u4 == 3)
                    u4 = 1;
                else
                    u4 = 0;

                if (u5 == 1 || u5 == 3)
                    u5 = 1;
                else
                    u5 = 0;

                if (u6 == 1 || u6 == 3)
                    u6 = 1;
                else
                    u6 = 0;

                if (u7 == 1 || u7 == 3)
                    u7 = 1;
                else
                    u7 = 0;
                
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

