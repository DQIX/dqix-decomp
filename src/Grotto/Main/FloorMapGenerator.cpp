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

// 1 and 3 correspond to blocked/void tiles, so this is replacing a tile value
// with a bool indicating whether it's blocked or not. Note that this is an
// idempotent operation, so when interpreting the effect of this you can ignore
// successive invocations on the same variable (at least for the purposes of
// function behaviour: they'll still affect the assembly output).
#define REDUCE_TILEVALUE(n) if (n == 1 || n == 3) n = 1; else n = 0

// USA: func_02091bc0
// JPN: func_020924e0
bool FloorMapGenerator::RoutineJ()
{    
    int upRoomIdx;
    Room room;
    
    int numAttempts = 0;
    int numDAttempts = 0;

    int posX, posY;
    while (true)
    {
        upRoomIdx = RandRange(0, roomCount - 1);
        room.CopyFrom(rooms[upRoomIdx]);
        posX = RandRange(room.bounds.left, room.bounds.right);
        posY = RandRange(room.bounds.top, room.bounds.bottom);
        int tileN, tileS, tileW, tileE;
        if (numAttempts < 100)
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
                numAttempts++;
                continue;
            }
        
            if ((tileN && tileE) || (tileN && tileW) ||
                (tileS && tileE) || (tileS && tileW))
            {
                numAttempts++;
                continue;
            }
            
            unsigned char adjacency;
            pFloorMap->ComputeSingleAdjacencyDataPoint(posX, posY, &adjacency);
            if (adjacency == 0xEA || adjacency == 0xAB || adjacency == 0xAE || adjacency == 0xBA ||
                 adjacency == 0xA3 || adjacency == 0x8E || adjacency == 0x3A || adjacency == 0xE8 ||
                 adjacency == 0xB8 || adjacency == 0xE2 || adjacency == 0x8B || adjacency == 0x2E)
            {
                numAttempts++;
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
        numAttempts = 0;
        Room* roomArray = this->rooms;
        
        while (true)
        {
            int downRoomIdx = RandRange(0, roomCount - 1);
            if (upRoomIdx == downRoomIdx && numDAttempts < 25)
            {
                numDAttempts++;
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
            numAttempts++;
            continue;
        }
    
        if ((tileN && tileE) || (tileN && tileW) ||
            (tileS && tileE) || (tileS && tileW))
        {
            numAttempts++;
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

