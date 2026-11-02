#pragma ipa file

#include "Graphics/OBJSprite.h"
#include "Resource/PAC.h"

extern char data_020f0504[]; // "RECN"
extern char data_020f0509[]; // "RNAN"
extern char data_020f050e[]; // "RGCN"
extern char data_020f0513[]; // "RLCN"

void OBJSprite::Metatile::Reset()
{
    memset(&nitroData_0, 0, sizeof(Nitro2DCellBank::Metatile));
    oamEntries_8 = NULL;
    unknown_c = unknown_10 = 1 << 12;
    unknown_14 = unknown_18 = 0;
    unknown_1c = 0;
    unknown_1e = 0;
    unknown_20 = 0;
    unknown_22 = 0;
    unknown_23 = 0;
    unknown_24 = 0;
    unknown_25 = -1;
    unknown_26 = -1;
}

bool GetCharBlockFromNitroFile(Nitro2DCharacter** out, void* data, unsigned int length)
{
    if (out == NULL)
        return true;
    if (data == NULL)
        return true;
    if (length == 0)
        return true;
    if (!Nitro2D_GetCharacter(data, out))
        return true;
    return false;
}

void OBJSprite::Reset()
{
    memset(&charLoad_0_, 0, sizeof(Nitro2DCharacterLoadOutput));
    memset(&paletteLoad_24_, 0, sizeof(Nitro2DPaletteLoadOutput));
    unknown_38_ = 0;
    metatiles_40_ = NULL;
    unknown_44_ = 0;
    unknown_48_ = 0;
    numMetatiles_4c_ = 0;
    unknown_4e_ = 0;
    unknown_3c_ = 0;
    unknown_50_ = true;
}

void OBJSprite::Destroy()
{
}

bool OBJSprite::LoadFromPAC(void* data, unsigned int length, SafeAllocator* alloc)
{
    if (length != 0)
    {
        int numFiles = GetNumFilesInPAC(data);
        for (int i = 0; i < numFiles; i++)
        {
            const char* innerName = NULL;
            unsigned int innerLength = 0;
            void* innerFile = GetFileInPACByIndex(data, i, &innerName, &innerLength);
            if (innerFile != NULL)
                LoadFromNitroFile(innerFile, innerLength, alloc);
        }
        return false; // wtf?
    }
    return true; // wtf?
}

bool OBJSprite::LoadFromNitroFile(void* data, unsigned int length, SafeAllocator* alloc)
{
    char signatureString[5] = {0};
    memcpy(signatureString, data, 4);
    if (memcmp(signatureString, data_020f0504, 4) == 0)
    {
        if (LoadNCER(data, length, alloc))
            return true;
    }
    if (memcmp(signatureString, data_020f0509, 4) == 0)
    {
        if (LoadNANR(data, length, alloc))
            return true;
    }
    else if (memcmp(signatureString, data_020f050e, 4) == 0)
    {
        if (LoadNCGR(data, length))
            return true;
    }
    else if (memcmp(signatureString, data_020f0513, 4) == 0)
    {
        if (LoadNCLR(data, length))
            return true;
    }

    return false;
}

bool OBJSprite::LoadNCER(void* argData, unsigned int length, SafeAllocator* alloc)
{
    void* data = argData;
    Nitro2DCellBank* cellBank;
    if (data == NULL)
        data = NULL; // yes, this is real code
    else if (length == 0)
        data = NULL;
    else
    {
        // ah yes. memcpy from data to data. my favorite
        memcpy(data, argData, length);
        if (!Nitro2D_GetCellBank(data, &cellBank))
            data = NULL;
    }

    if (data == NULL)
        return true;

    if (alloc != NULL && metatiles_40_ == NULL)
    {
        numMetatiles_4c_ = cellBank->numMetatiles_0;
        metatiles_40_ = (Metatile*)alloc->Allocate(numMetatiles_4c_ * sizeof(Metatile));
        for (unsigned short i = 0; i < numMetatiles_4c_; i++)
            metatiles_40_[i].Reset();
    }

    Metatile* pMetatiles = metatiles_40_;
    Metatile* localMetatile;
    unsigned short numMetatiles = numMetatiles_4c_;
    
    if (pMetatiles != NULL && numMetatiles != 0 && alloc != NULL)
    {
        unsigned short sourceNumMetatiles = cellBank->numMetatiles_0;
        Nitro2DCellBank::Metatile* nitroMetatile;
        unsigned short i;
        unsigned short objCounter = 0;
        for (i = 0; i < sourceNumMetatiles; i++)
        {
            if (numMetatiles <= i)
                break;
            nitroMetatile = Nitro2D_GetCellBankMetatile(cellBank, i);
            if (nitroMetatile == NULL)
                break;
            unsigned short thisNumObjs = nitroMetatile->numObjs_0;
            localMetatile = &pMetatiles[i];
            localMetatile->unknown_1e = i;
            localMetatile->unknown_22 = objCounter;
            localMetatile->nitroData_0 = *nitroMetatile;
            localMetatile->nitroData_0.pAttributes_4 = localMetatile->oamEntries_8 =
                (OAMEntry*)alloc->Allocate(thisNumObjs * sizeof(OAMEntry));
            for (unsigned short j = 0; j < nitroMetatile->numObjs_0; j++)
            {
                localMetatile->oamEntries_8[j] = nitroMetatile->pAttributes_4[j];
            }
            objCounter += nitroMetatile->numObjs_0;
        }
    }

    unknown_4e_ = cellBank->numMetatiles_0;
    unknown_38_ = cellBank->unknown_8;

    Nitro2DCellBank::Metatile* lastNitroMT = Nitro2D_GetCellBankMetatile(cellBank, cellBank->numMetatiles_0 - 1);
    if (lastNitroMT == NULL)
        return true;
    unsigned int shift = unknown_38_;
    OAMEntry* lastOAM = &lastNitroMT->pAttributes_4[lastNitroMT->numObjs_0 - 1];
    
    int word2 = lastOAM->word2;
    int word1 = lastOAM->word1;
    int word0 = lastOAM->word0;
    shift += 5;
    unsigned int shiftedTileNum = (word2 & 0x3ff) << (shift);
    unsigned int sizeIndex = (word1 & 0xc000) >> 14;
    unsigned int shapeIndex = (word0 & 0xc000) >> 14;
    unsigned int minShift = 5;
    switch (shapeIndex)
    {
    case 0: // square
        switch (sizeIndex)
        {
        case 0: // 8x8
            minShift = 5; break;
        case 1: // 16x16
            minShift = 7; break;
        case 2: // 32x32
            minShift = 9; break;
        case 3: // 64x64
            minShift = 11; break;
        }
        break;
    case 1: // horizontal
        switch (sizeIndex)
        {
        case 0: // 16x8
            minShift = 6; break;
        case 1: // 32x8
            minShift = 7; break;
        case 2: // 32x16
            minShift = 8; break;
        case 3: // 64x32
            minShift = 10; break;
        }
        break;
    case 2: // vertical
        switch (sizeIndex)
        {
        case 0: // 8x16
            minShift = 6; break;
        case 1: // 8x32
            minShift = 7; break;
        case 2: // 16x32
            minShift = 8; break;
        case 3: // 32x64
            minShift = 10; break;
        }
        break;
    }
    if (shift < minShift)
        shift = minShift;
    unknown_48_ = shiftedTileNum + (1 << shift);
    return false;
}

