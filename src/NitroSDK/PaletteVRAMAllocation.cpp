#include "NitroSDK/VRAMAllocation.h"

#ifdef jpn
#define data_0210cf88 data_0210cc40
#endif

struct PaletteAllocationData
{
    unsigned int freeStart;
    unsigned int freeEnd;
    unsigned int totalLength;
} static data_0210cf88;

int FrameFreeTexturePaletteVRAM(unsigned int);
void ResetTexturePaletteAllocationState();

void InitializeTexturePaletteVRAMAllocation(unsigned int size, CBool setAllocFunctions)
{
    data_0210cf88.totalLength = size;
    ResetTexturePaletteAllocationState();
    if (setAllocFunctions)
    {
        g_pfnAllocTexturePalette = &FrameAllocateTexturePaletteVRAM;
        g_pfnFreeTexturePalette = &FrameFreeTexturePaletteVRAM;
    }
}

inline CBool FrameAllocateTexturePalette_Front(unsigned int size, CBool eightByte, unsigned int* pOutcome)
{
    unsigned int freeStart = data_0210cf88.freeStart;
    unsigned int distanceToAligned;
    if (eightByte)
        distanceToAligned = (8 - (freeStart & 7)) & 7;
    else
        distanceToAligned = (16 - (freeStart & 15)) & 15;
    unsigned int neededSize = size + distanceToAligned;

    if (data_0210cf88.freeEnd - data_0210cf88.freeStart >= neededSize)
    {
        unsigned int allocEnd = freeStart + neededSize;
        if (eightByte && allocEnd > 0x10000)
            return false;

        *pOutcome = freeStart + distanceToAligned;
        data_0210cf88.freeStart += neededSize;
        return true;
    }
    else
        return false;
}

inline CBool FrameAllocateTexturePalette_Back(unsigned int size, CBool eightByte, unsigned int* pOutcome)
{
    if (data_0210cf88.freeEnd >= size)
    {
        unsigned int freeStart = data_0210cf88.freeEnd - size;
        unsigned int alignmentWaste;
        if (eightByte)
            alignmentWaste = freeStart & 7;
        else
            alignmentWaste = freeStart & 15;
        unsigned int neededSize = size + alignmentWaste;

        if (data_0210cf88.freeEnd - data_0210cf88.freeStart >= neededSize)
        {
            unsigned int freeEnd = data_0210cf88.freeEnd;
            if (eightByte && freeEnd > 0x10000)
                return false;
            
            data_0210cf88.freeEnd -= neededSize;
            *pOutcome = data_0210cf88.freeEnd;
            return true;
        }
    }
    return false;
}

unsigned int FrameAllocateTexturePaletteVRAM(unsigned int size, CBool eightByte, int allocDir)
{
    unsigned int chosenOffset = 0;
    CBool success = false;

    size = (size == 0) ? 8 : ((size + 7) & ~7);

    if (size >= 0x7fff8) // 512kb
        return 0;

    if (allocDir == 1) // allocate at front
        success = FrameAllocateTexturePalette_Front(size, eightByte, &chosenOffset);
    else
        success = FrameAllocateTexturePalette_Back(size, eightByte, &chosenOffset);

    if (success)
        return ((size >> 3) << 16) | ((chosenOffset >> 3) & 0xffff);
    return 0;
}

int FrameFreeTexturePaletteVRAM(unsigned int)
{
    return 0;
}

void SaveTexturePaletteAllocationState(unsigned int* pRange)
{
    pRange[0] = data_0210cf88.freeStart;
    pRange[1] = data_0210cf88.freeEnd;
}

void RestoreTexturePaletteAllocationState(const unsigned int* pRange)
{
    data_0210cf88.freeStart = pRange[0];
    data_0210cf88.freeEnd = pRange[1];
}

void ResetTexturePaletteAllocationState()
{
    data_0210cf88.freeStart = 0;
    data_0210cf88.freeEnd = data_0210cf88.totalLength;
}