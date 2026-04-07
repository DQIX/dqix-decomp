#include <globaldefs.h>
#include "Grotto/TileFeatures.h"

extern "C" extern char* strstr(const char*, const char*);
#ifdef jpn
#define data_020e6f04 data_020e77a8
#define data_020e6f00 data_020e77a4
#define data_020ef292 data_020ef1ce
#endif
extern char const data_020e6f04[];
extern char const data_020e6f00[];
extern char const data_020ef292[];


int GenerateGrottoObjectPositionOrientation(void* data, int tileX, int tileY, int* outX, int* outY,
    const TileFeaturePlacementData* tileDataArray, bool preferFaceDown) {
    const TileFeaturePlacementData* tileDataPtr;
    if (tileDataArray == 0)
    {
        // This is absolutely hideous, but it produces the right assembly
        char* ptr1 = *(char**)((char*)data + 0x420);
        char* ptr2 = ptr1 + tileX * 0x48 + (tileY << 4) * 0x48;
        if (ptr2 == 0)
            return -1;
        tileDataPtr = (const TileFeaturePlacementData*)(ptr2 + 4);
    }
    else
    {
        // Same here, obviously we're indexing into the array, but this
        // way produces the correct assembly
        tileDataPtr = (const TileFeaturePlacementData*)(
            (const char*)tileDataArray + 0x10 * tileX + 0x100 * tileY);
    }
    int position = ChooseTileFeaturePosition(tileDataPtr);
    int direction = ChooseTileFeatureOrientation(tileDataPtr, position, preferFaceDown);
    if (position <= 8)
    {
        // These arrays hold 0 or +/- 10922. We have to reference them as
        // char pointers too or we get the wrong assembly
        *outX = *(const int*)(data_020e6f00 + 8 * position);
        *outY = *(const int*)(data_020e6f04 + 8 * position);
    }
    // data_020ef292 holds "R02A"
    char* strMatch = strstr(tileDataPtr->tilename, data_020ef292);
    if (strMatch != 0)
    {
        if (position == 0)
        {
            *outX = -0x34cc;
            *outY = -0x34cc;
        }
        else if (position == 2)
        {
            *outX = 0x34cc;
            *outY = -0x34cc;
        }
        else if (position == 6)
        {
            *outX = -0x34cc;
            *outY = 0x34cc;
        }
        else if (position == 8)
        {
            *outX = 0x34cc;
            *outY = 0x34cc;
        }
    }
    *outX += (int)((float)tileX * 8.0f * 4096.0f);
    *outY += (int)((float)tileY * 8.0f * 4096.0f);
    return direction;
}