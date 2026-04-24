#include "Grotto/Main/TileFeatures.h"
#include "std_library_functions.h"
//#include "padsection.h"

#ifdef jpn
#define data_020e7010 data_020e78b4
#define func_02012fe4 func_02012dac
#define data_020e700c data_020e78b0
#endif

extern unsigned char const data_020e7010[];
extern "C" int func_02012fe4(void);
extern unsigned char const data_020e700c[];

// USA: func_0201ea54
// JPN: func_0201e7e0
int ChooseTileFeaturePosition(const TileFeaturePlacementData* data)
{
    unsigned char validArray[12];
 
    // This function doesn't seem to do anything besides put a value in r0.
    // We never use the value, but it gets called so yeah   
    func_02012fe4();

    int validCount = 0;

    int j = 9;
    unsigned char* copyDst = validArray;
    const unsigned char* copySrc = data_020e7010;
    do
    {
        j--;
        *copyDst = *copySrc;
        copyDst++;
        copySrc++;
    } while (j != 0);

    for (int i = 0; i < 9; i++)
    {
        if (data->directionBitmasks[i] != 0)
        {
            validArray[validCount++] = i;
        }
    }
    
    return (int)(char)validArray[rand() % validCount];
}

// USA: func_0201ead0
// JPN: func_0201e85c
int ChooseTileFeatureOrientation(const TileFeaturePlacementData* data,
    int direction, bool preferFaceDown)
{
    char validDirections[4];

    func_02012fe4();
    int numValidDirections = 0;    
    int j = 4;
    unsigned char* dstPtr = (unsigned char*)validDirections;
    const unsigned char* srcPtr = data_020e700c;
    do {
        j--;
        *dstPtr = *srcPtr;
        dstPtr++;
        srcPtr++;
    } while (j != 0);

    for (unsigned int i = 0; (int)i < 4; i++)
    {
        if (((int)data->directionBitmasks[direction] >> i & 1u) != 0)
        {
            validDirections[numValidDirections++] = i;
        }
    }

    if (preferFaceDown)
    {
        // 2 = facing down, 1 = facing right, 3 = facing left
        for (int i = 0; i < numValidDirections; i++)
        {
            if (validDirections[i] == 2)
                return validDirections[i];
        }

        for (int i = 0; i < numValidDirections; i++)
        {
            if (validDirections[i] == 1)
                return validDirections[i];
        }

        for (int i = 0; i < numValidDirections; i++)
        {
            if (validDirections[i] == 3)
                return validDirections[i];
        }
    }

    return (int)validDirections[rand() % numValidDirections];
}
