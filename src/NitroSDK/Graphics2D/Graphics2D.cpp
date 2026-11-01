#include "NitroSDK/Graphics2D/Graphics2D.h"
#include <globaldefs.h>

extern "C"
{
}

#define SIGNATURE_ABNK 0x41424e4b
#define SIGNATURE_CEBK 0x4345424b
#define SIGNATURE_CHAR 0x43484152
#define SIGNATURE_PLTT 0x504c5454

template<class T>
static inline T* FixPointerOffsetRight(T** pPtr, void* base)
{
    return (*pPtr = (T*)((intptr_t)*pPtr + (intptr_t)base));
}

template<class T>
static inline T* FixPointerOffsetLeft(T** pPtr, void* base)
{
    return (*pPtr = (T*)((intptr_t)base + (intptr_t)*pPtr));
}

void InternalPrepareCellBankSubstruct14(Nitro2DCellBank::Substruct14::Interior* ptr);
void PrepareABNK(Nitro2DAnimationBank* ptr);
void PrepareCEBK(Nitro2DCellBank* ptr);
void PrepareCHAR(Nitro2DCharacter* ptr);
void PreparePLTT(Nitro2DPalette* ptr);

CBool InternalGetAnimationBank(void* fileData, Nitro2DAnimationBank** output)
{
    Nitro2DInnerFileHeader* fileHeader = Nitro2D_GetInnerFileBySignature(fileData, SIGNATURE_ABNK);
    if (fileHeader == NULL)
    {
        *output = NULL;
        return false;
    }
    PrepareABNK((Nitro2DAnimationBank*)fileHeader->GetInterior());
    *output = (Nitro2DAnimationBank*)fileHeader->GetInterior();
    return true;
}

CBool Nitro2D_GetAnimationBank(void* fileData, Nitro2DAnimationBank** output)
{
    return InternalGetAnimationBank(fileData, output);
}

void PrepareABNK(Nitro2DAnimationBank* ptr)
{
    FixPointerOffsetRight(&ptr->pAnimBlocks_4, ptr);
    FixPointerOffsetRight(&ptr->pFrameBlocks_8, ptr);
    FixPointerOffsetRight(&ptr->pFrameData_c, ptr);

    unsigned short blockIndex;
    unsigned short frameIndex;
    uint16_t* frameData = ptr->pFrameData_c;
    Nitro2DAnimationBank::AnimationBlock* animBlocks = ptr->pAnimBlocks_4;
    Nitro2DAnimationBank::FrameBlock* frameBlocks = ptr->pFrameBlocks_8;

    for (blockIndex = 0; ptr->numAnimBlocks_0 > blockIndex; blockIndex++)
    {
        FixPointerOffsetLeft(&animBlocks[blockIndex].unknown_c, frameBlocks);
        for (frameIndex = 0; animBlocks[blockIndex].numFrames_0 > frameIndex; frameIndex++)
        {
            FixPointerOffsetLeft(&animBlocks[blockIndex].unknown_c[frameIndex].unknown_0, frameData);
        }
    }

    if (ptr->unknown_14 == NULL)
        return;

    FixPointerOffsetRight(&ptr->unknown_14, ptr);
    Nitro2DAnimationBank::Subsubstruct* subsub = &ptr->unknown_14->unknown_8;
    FixPointerOffsetRight(&subsub->unknown_4, subsub);
    for (unsigned int i = 0; i < subsub->unknown_0; i++)
    {
        unsigned int j;
        Nitro2DAnimationBank::Subsubsub* sss = &subsub->unknown_4[i];
        FixPointerOffsetRight(&sss->unknown_4, subsub);
        FixPointerOffsetRight(&sss->unknown_8, subsub);
        for (j = 0; j < sss->unknown_0; j++)
        {
            FixPointerOffsetRight(&sss->unknown_8[j], subsub);
        }
    }
}

Nitro2DAnimationBank::AnimationBlock* Nitro2D_AnimationBankGetAnimationBlock(Nitro2DAnimationBank* bank, unsigned int idx)
{
    if (bank->numAnimBlocks_0 > idx)
        return bank->pAnimBlocks_4 + idx;
    return NULL;
}

Nitro2DCellBank::OBJAttributes* GetCellBankOBJAttributes(Nitro2DCellBank* bank)
{
    // OBJ attributes begin at the end of the metatile table
    if (bank->metatileEntrySize_2 & 1)
        return (Nitro2DCellBank::OBJAttributes*)((Nitro2DCellBank::MetatileEXT*)bank->pMetatiles_4 + bank->numMetatiles_0);
    else
        return (Nitro2DCellBank::OBJAttributes*)(bank->pMetatiles_4 + bank->numMetatiles_0);
}

void PrepareCellBankSubstruct14(Nitro2DCellBank::Substruct14* ptr)
{
    InternalPrepareCellBankSubstruct14(&ptr->interior);
}

CBool Nitro2D_GetCellBank(void* fileData, Nitro2DCellBank** output)
{
    Nitro2DInnerFileHeader* fileHeader = Nitro2D_GetInnerFileBySignature(fileData, SIGNATURE_CEBK);
    if (fileHeader == NULL)
    {
        *output = NULL;
        return false;
    }
    PrepareCEBK((Nitro2DCellBank*)fileHeader->GetInterior());
    *output = (Nitro2DCellBank*)fileHeader->GetInterior();
    return true;
}

Nitro2DCellBank::Metatile* Nitro2D_GetCellBankMetatile(Nitro2DCellBank* bank, unsigned int idx)
{
    if (idx >= bank->numMetatiles_0)
        return NULL;
    if (bank->metatileEntrySize_2 & 1)
        return (Nitro2DCellBank::Metatile*)((Nitro2DCellBank::MetatileEXT*)bank->pMetatiles_4 + idx);
    else
        return bank->pMetatiles_4 + idx;
}

void PrepareCEBK(Nitro2DCellBank* ptr)
{
    FixPointerOffsetRight(&ptr->pMetatiles_4, ptr);
    Nitro2DCellBank::OBJAttributes* oam = GetCellBankOBJAttributes(ptr);

    for (unsigned short i = 0; i < ptr->numMetatiles_0; i++)
    {
        Nitro2DCellBank::Metatile* metatile = Nitro2D_GetCellBankMetatile(ptr, i);
        FixPointerOffsetRight(&metatile->pAttributes_4, oam);
    }

    if (ptr->unknown_c != NULL)
    {
        Nitro2DCellBank::SubstructC* subc = FixPointerOffsetRight(&ptr->unknown_c, ptr);
        FixPointerOffsetRight(&subc->unknown_4, subc);
        ptr->unknown_c = subc; // why write this twice?
    }

    if (ptr->unknown_14 != NULL)
    {
        FixPointerOffsetRight(&ptr->unknown_14, ptr);
        PrepareCellBankSubstruct14(ptr->unknown_14);
    }
}

CBool Nitro2D_GetCharacter(void* fileData, Nitro2DCharacter** output)
{
    Nitro2DInnerFileHeader* fileHeader = Nitro2D_GetInnerFileBySignature(fileData, SIGNATURE_CHAR);
    if (fileHeader == NULL)
    {
        *output = NULL;
        return false;
    }
    PrepareCHAR((Nitro2DCharacter*)fileHeader->GetInterior());
    *output = (Nitro2DCharacter*)fileHeader->GetInterior();
    return true;
}

void PrepareCHAR(Nitro2DCharacter* ptr)
{
    FixPointerOffsetRight(&ptr->pTileData_14, ptr);
}

CBool Nitro2D_GetPalette(void* fileData, Nitro2DPalette** output)
{
    Nitro2DInnerFileHeader* fileHeader = Nitro2D_GetInnerFileBySignature(fileData, SIGNATURE_PLTT);
    if (fileHeader == NULL)
    {
        *output = NULL;
        return false;
    }
    PreparePLTT((Nitro2DPalette*)fileHeader->GetInterior());
    *output = (Nitro2DPalette*)fileHeader->GetInterior();
    return true;
}

void PreparePLTT(Nitro2DPalette* ptr)
{
    FixPointerOffsetRight(&ptr->pPaletteData_c, ptr);
}

Nitro2DInnerFileHeader* Nitro2D_GetInnerFileBySignature(void* fileData, uint32_t signature)
{
    Nitro2DFileHeader* header = (Nitro2DFileHeader*)fileData;
    Nitro2DInnerFileHeader* innerFile = header->GetFirstSubfile();
    for (unsigned short i = 0; header->numSubfiles > i; i++)
    {
        if (innerFile->signature == signature)
            return innerFile;
        innerFile = innerFile->Next();
    }
    return NULL;
}

void InternalPrepareCellBankSubstruct14(Nitro2DCellBank::Substruct14::Interior* ptr)
{
    FixPointerOffsetRight(&ptr->pointers_4, ptr);
    for (unsigned short i = 0; i < ptr->numPointers_0; i++)
    {
        FixPointerOffsetLeft(&ptr->pointers_4[i], ptr);
    }
}