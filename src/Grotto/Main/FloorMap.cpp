#include "Grotto/Main/FloorMap.h"
#include "std_library_functions.h"
#include <globaldefs.h>

// USA: func_02092644
// JPN: func_02092f64
ARM bool FloorMap::Initialize(int w, int h)
{
    if (w < 8 || h < 8)
        return false;
    if (w > 16 || h > 16)
        return false;

    width = w;
    height = h;
    InitBuffers();
    return true;
}

// USA: func_0209267c
// JPN: func_02092f9c
ARM void FloorMap::InitBuffers()
{
    if (pMapData != 0)
        memset(pMapData, 1, 256);
    if (pMapAdjacencyData != 0)
        memset(pMapAdjacencyData, 0xff, 256);
}

// important not to have brackets around this, and to use in pointer
// arithmetic instead of an array index, otherwise the additions are carried
// out in reverse order (different assembly output)
#define TILE_OFFSET(x, y) ((y) << 4) + (x)

// USA: func_020926b8
// JPN: func_02092fd8
ARM bool FloorMap::ComputeAdjacencyData()
{
    for (int y = 1; y < 15; y++)
    {
        for (int x = 1; x < 15; x++)
        {
            ComputeSingleAdjacencyDataPoint(x, y, 
                pMapAdjacencyData + TILE_OFFSET(x, y));
        }
    }

    for (int y = 0; y < height; y++)
    {
        *(pMapAdjacencyData + TILE_OFFSET(0, y)) = 0xFF;
        *(pMapAdjacencyData + TILE_OFFSET(width - 1, y)) = 0xFF;
    }

    for (int x = 0; x < width; x++)
    {
        *(pMapAdjacencyData + TILE_OFFSET(x, 0)) = 0xFF;
        *(pMapAdjacencyData + TILE_OFFSET(x, height - 1)) = 0xFF;
    }

    return true;
}

// USA: func_0209277c
// JPN: func_0209309c
ARM void FloorMap::ComputeSingleAdjacencyDataPoint(int x, int y, unsigned char* outData)
{
    unsigned char thisTile = *(pMapData + TILE_OFFSET(x, y));

    if (thisTile == 1 || thisTile == 3)
    {
        *outData = 0xFF;
        return;
    }

    unsigned char tileNW = *(pMapData + TILE_OFFSET(x - 1, y - 1));
    unsigned char tileN = *(pMapData + TILE_OFFSET(x, y - 1));
    unsigned char tileNE = *(pMapData + TILE_OFFSET(x + 1, y - 1));
    unsigned char tileE = *(pMapData + TILE_OFFSET(x + 1, y));
    unsigned char tileSE = *(pMapData + TILE_OFFSET(x + 1, y + 1));
    unsigned char tileS = *(pMapData + TILE_OFFSET(x, y + 1));
    unsigned char tileSW = *(pMapData + TILE_OFFSET(x - 1, y + 1));
    unsigned char tileW = *(pMapData + TILE_OFFSET(x - 1, y));

    if (thisTile != 0 && thisTile != 2 && (unsigned char)(thisTile + 0xfc) > 4)
        return;
    
    if (tileNW == 1 || tileNW == 3)
        *outData |= 0x80;
    else
        *outData &= ~0x80;

    if (tileNE == 1 || tileNE == 3)
        *outData |= 0x20;
    else
        *outData &= ~0x20;

    if (tileSE == 1 || tileSE == 3)
        *outData |= 0x08;
    else
        *outData &= ~0x08;

    if (tileSW == 1 || tileSW == 3)
        *outData |= 0x02;
    else
        *outData &= ~0x02;

    if (tileN == 1 || tileN == 3)
    {
        *outData |= 0x80;
        *outData |= 0x40;
        *outData |= 0x20;
    }
    else
        *outData &= ~0x40;

    if (tileE == 1 || tileE == 3)
    {
        *outData |= 0x20;
        *outData |= 0x10;
        *outData |= 0x08;
    }else
        *outData &= ~0x10;

    if (tileS == 1 || tileS == 3)
    {
        *outData |= 0x08;
        *outData |= 0x04;
        *outData |= 0x02;
    }
    else
        *outData &= ~0x04;

    
    if (tileW == 1 || tileW == 3)
    {
        *outData |= 0x02;
        *outData = (*outData & ~0x01) | 0x01;
        *outData |= 0x80;
    }
    else
        *outData &= ~0x01;
}

// USA: func_0209291c
// JPN: func_0209323c
ARM void FloorMap::WriteTile(int x, int y, unsigned char value)
{
    if (x >= 0 && y >= 0)
        *(pMapData + TILE_OFFSET(x, y)) = value;
}

// USA: func_02092934
// JPN: func_02093254
ARM int FloorMap::GetTile(int x, int y) const
{
    if (x < 0 || y < 0)
        return -1;
    return (int)*(pMapData + TILE_OFFSET(x, y));
}

// USA: func_02092950
// USA: func_02093270
ARM void FloorMap::WriteRectangle(int left, int top, int width, int height, unsigned char value)
{
    if (left < 0 || top < 0 || width < 1 || height < 1)
        return;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            *(pMapData + TILE_OFFSET(left + x, top + y)) = value;
        }
    }
}

// USA: func_020929b4
// JPN: func_020932d4
ARM int FloorMap::GetAdjacencyBits(int x, int y) const
{
    if (x < 0 || y < 0)
    {
        volatile char notUnusualAtAll = 0;
        return 0;
    }
    
    return (int)(char)*(pMapAdjacencyData + TILE_OFFSET(x, y));
}

// USA: func_020929e4
// JPN: func_02093304
ARM void FloorMap::AllocateBuffers(SafeAllocator* allocator)
{
    if (pMapData == 0)
        pMapData = (unsigned char*)allocator->Allocate(256);

    if (pMapAdjacencyData == 0)
        pMapAdjacencyData = (unsigned char*)allocator->Allocate(256);
}

// USA: func_02092a2c
// JPN: func_0209334c
ARM void FloorMap::Clear()
{
    width = 0;
    height = 0;
    stairsUp.x = 0;
    stairsUp.y = 0;
    stairsDown.x = 0;
    stairsDown.y = 0;

    chestCount = 0;
    pMapData = NULL;
    pMapAdjacencyData = NULL;
    memset(chests, 0, 10);
}

// USA: func_02092a68
// JPN: func_02093388
ARM void FloorMap::Clear2()
{
    Clear();
}