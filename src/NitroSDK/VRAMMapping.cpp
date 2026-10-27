#pragma ipa file

#include <std_library_functions.h>
#include <globaldefs.h>

typedef unsigned int (*Alloc588Proc)(unsigned int, CBool, int);
typedef int (*Free588Proc)(unsigned int);
typedef unsigned int (*Alloc7ccProc)(unsigned int, CBool, int);
typedef int (*Free7ccProc)(unsigned int);

// sizeof == 0x18
struct AllocationObject
{
    unsigned int start_0;
    unsigned int end_4;
    int unknown_8;
    int unknown_c;
    unsigned short unknown_10;
    unsigned short unknown_12;
    unsigned int unknown_14;

    inline unsigned int GetLength() const
    {
        return end_4 - start_0;
    }
};

extern unsigned short data_0210cf84;
struct Struct_0210cf88
{
    unsigned int unknown_0;
    unsigned int unknown_4;
    unsigned int unknown_8;

    inline unsigned int GetLength()
    {
        return unknown_4 - unknown_0;
    }

} extern data_0210cf88;

extern AllocationObject data_020f1f14[5];

extern Alloc588Proc data_020f1ee8;
extern Free588Proc data_020f1eec;
extern Alloc7ccProc data_020f1ef0;
extern Free7ccProc data_020f1ef4;

extern AllocationObject* data_020f1ef8[7];

inline AllocationObject* Foo(int i)
{
    AllocationObject* const* x = data_020f1ef8 + 2;
    return x[i];
}

void PrepareAllocationObjects_588();
unsigned int Allocate_020bb588(unsigned int requestedSize, CBool arg31, int unused);
int Free_588(unsigned int);

unsigned int Allocate_020bb7cc(unsigned int requestedSize, CBool arg31, int unused);
int Free_7cc(unsigned int);

void PrepareRange_7cc();

unsigned int Alloc588Proc_Invalid(unsigned int length, CBool arg31, int unused)
{
    return 0;
}

int Free588Proc_Invalid(unsigned int)
{
    return -1;
}

unsigned int Alloc7ccProc_Invalid(unsigned int length, CBool arg31, int unused)
{
    return 0;
}

int Free7ccProc_Invalid(unsigned int)
{
    return -1;
}

static void SetImageAllocationOrder(int first, int second, int third, int fourth, int fifth)
{
    data_020f1ef8[2] = &data_020f1f14[first];
    data_020f1ef8[3] = &data_020f1f14[second];
    data_020f1ef8[4] = &data_020f1f14[third];
    data_020f1ef8[5] = &data_020f1f14[fourth];
    data_020f1ef8[6] = &data_020f1f14[fifth];
}

void MakeVRAMBanksAvailable_588(unsigned int numBanks, CBool setAllocFunctions)
{
    if (numBanks <= 2)
        SetImageAllocationOrder(4, 3, 2, 0, 1);
    else
        SetImageAllocationOrder(4, 3, 0, 2, 1);

    data_0210cf84 = numBanks;
    PrepareAllocationObjects_588();

    if (setAllocFunctions)
    {
        data_020f1ee8 = &Allocate_020bb588;
        data_020f1eec = &Free_588;
    }
}

void PrepareAllocationObjects_588()
{
    int i;
    AllocationObject* loopObj = data_020f1f14;
    int numberThreshold = data_0210cf84;
    if (data_0210cf84 > 1)
        numberThreshold++;
    for (i = 0; i < 5; i++, loopObj++)
    {
        if (i < numberThreshold)
            loopObj->unknown_8 = 1;
        else
            loopObj->unknown_8 = 0;

        if (loopObj->unknown_c != 0)
        {
            loopObj->start_0 = 0;
            loopObj->end_4 = 0x10000; // 64k
        }
        else
        {
            loopObj->start_0 = 0;
            loopObj->end_4 = 0x20000; // 128k
        }
    }
}

unsigned int Allocate_020bb588(unsigned int requestedSize, CBool arg31, int unused)
{
    unsigned int chosenOffset;
    unsigned int size;
    
    if (requestedSize == 0)
        size = 16;
    else
        size = (requestedSize + 15) & ~15;

    if (size >= 0x7fff0) // 512kb, VRAM banks A-D?
        return 0;

    CBool success;
    if (arg31)
    {
        AllocationObject* objA;
        AllocationObject* objB;
        for (int i = 0; i < 2; i++)
        {
            objB = data_020f1ef8[i];
            if (objB->unknown_8 == 0 || objB->GetLength() < size)
                continue;
            
            switch (objB->unknown_10)
            {
            case 0:
                objA = &data_020f1f14[1];
                break;
            case 3:
                objA = &data_020f1f14[2];
                break;
            default:
                objA = NULL;
                break;
            }

            if (objA->unknown_8 == 0 || objA->GetLength() < (size / 2))
                continue;

            unsigned int oldStart = objB->start_0;
            objB->start_0 += size;
            objA->start_0 += size / 2;
            chosenOffset = oldStart + objB->unknown_14;
            success = true;
            goto complete;
        }
        success = false;
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            AllocationObject* obj = Foo(i);
            if (obj->unknown_8 == NULL || obj->GetLength() < size)
                continue;
            obj->end_4 -= size;
            chosenOffset = obj->end_4 + obj->unknown_14;
            success = true;
            goto complete;
        }
        success = false;
    }

complete:
    if (!success)
        return 0;
    unsigned short numQwords = chosenOffset / 8;
    return ((size >> 4) << 16) | numQwords | (arg31 << 31);
}

int Free_588(unsigned int)
{
    return 0;
}

void SaveAllocationObjectState(unsigned int* output)
{
    int i;
    AllocationObject* loopObject = data_020f1f14;
    for (i = 0; i < 5; loopObject++, i++)
    {
        *(output + 2*i) = loopObject->start_0;
        *(output + 2*i + 1) = loopObject->end_4;
    }
}

void RestoreAllocationObjectState(const unsigned int* input)
{
    int i = 0;
    int twoi = 0;
    AllocationObject* loopObject = data_020f1f14;
    for (; i < 5; twoi += 2, i++, loopObject++)
    {
        loopObject->start_0 = *(input + twoi);
        loopObject->end_4 = *(input + twoi + 1);
    }
}

void MakeVRAMAvailable_7cc(unsigned int amount, CBool setAllocFunctions)
{
    data_0210cf88.unknown_8 = amount;
    PrepareRange_7cc();
    if (setAllocFunctions)
    {
        data_020f1ef0 = &Allocate_020bb7cc;
        data_020f1ef4 = &Free_7cc;
    }
}

inline CBool Allocate_020bb7cc_Front(unsigned int size, CBool eightByte, unsigned int* pOutcome)
{
    unsigned int freeStart = data_0210cf88.unknown_0;
    unsigned int distanceToAligned;
    if (eightByte)
        distanceToAligned = (8 - (freeStart & 7)) & 7;
    else
        distanceToAligned = (16 - (freeStart & 15)) & 15;
    unsigned int neededSize = size + distanceToAligned;

    if (data_0210cf88.GetLength() >= neededSize)
    {
        unsigned int newStart = freeStart + neededSize;
        if (eightByte && newStart > 0x10000)
            return false;

        *pOutcome = freeStart + distanceToAligned;
        data_0210cf88.unknown_0 += neededSize;
        return true;
    }
    else
        return false;
}

inline CBool Allocate_020bb7cc_Back(unsigned int size, CBool eightByte, unsigned int* pOutcome)
{
    if (data_0210cf88.unknown_4 >= size)
    {
        unsigned int freeStart = data_0210cf88.unknown_4 - size;
        unsigned int alignmentWaste;
        if (eightByte)
            alignmentWaste = freeStart & 7;
        else
            alignmentWaste = freeStart & 15;
        unsigned int neededSize = size + alignmentWaste;

        if (data_0210cf88.GetLength() >= neededSize)
        {
            unsigned int freeEnd = data_0210cf88.unknown_4;
            if (eightByte && freeEnd > 0x10000)
                return false;
            
            data_0210cf88.unknown_4 -= neededSize;
            *pOutcome = data_0210cf88.unknown_4;
            return true;
        }
    }
    return false;
}

unsigned int Allocate_020bb7cc(unsigned int size, CBool eightByte, int arg3)
{
    unsigned int chosenOffset = 0;
    CBool success = false;

    size = (size == 0) ? 8 : ((size + 7) & ~7);

    if (size >= 0x7fff8) // 512kb
        return 0;

    if (arg3 == 1)
    {
        success = Allocate_020bb7cc_Front(size, eightByte, &chosenOffset);
    }
    else
    {
        success = Allocate_020bb7cc_Back(size, eightByte, &chosenOffset);
    }
complete:

    if (success)
        return ((size >> 3) << 16) | ((chosenOffset >> 3) & 0xffff);
    return 0;
}

int Free_7cc(unsigned int)
{
    return 0;
}

void GetRange_7cc(unsigned int* pRange)
{
    pRange[0] = data_0210cf88.unknown_0;
    pRange[1] = data_0210cf88.unknown_4;
}

void SetRange_7cc(const unsigned int* pRange)
{
    data_0210cf88.unknown_0 = pRange[0];
    data_0210cf88.unknown_4 = pRange[1];
}

void PrepareRange_7cc()
{
    data_0210cf88.unknown_0 = 0;
    data_0210cf88.unknown_4 = data_0210cf88.unknown_8;
}