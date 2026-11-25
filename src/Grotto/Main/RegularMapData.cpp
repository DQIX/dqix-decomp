#include "Grotto/Main/DetailedTreasureMapData.h"
#include "Grotto/Main/TreasureMapDataStructs.h"
#include "GameState/GameState.h"
#include "Resource/GameStrings.h"
#include "NitroSDK/Memory.h"

extern "C"
{
    // seems to get the game language. In the USA version, if it would
    // return a value other than 2 or 5, it returns 1, which seems to reflect
    // lack of support for German & Italian.
    // not used in jpn version
    int func_0200fb08(GameState*);
}

#define BINARY_READ_AND_ADVANCE(buffer, offset, dst, len) \
    (VectorizedInvertedMemcpy((buffer) + (offset), (dst), (len)), offset += (len))

#define TMAPLANGDATA_READ(offset, into, len) \
    BINARY_READ_AND_ADVANCE(GameState::GetInstance()->GetTreasureMapLanguageData(), offset, into, len)

// USA: func_020a4618
// JPN: func_020a63b0
void DetailedTreasureMapData::RegularMapData::GenerateUnknownData()
{
    if (GameState::GetInstance()->GetTreasureMapLanguageData() == NULL)
        return;

    unsigned short numValues = 0;
    unsigned char index = 0;
    unsigned char value = 1;
    unsigned char chance = 0;

    unknown_66_ = 1;
    unknown_4f_ = 0;

    int readOffset = GetGameResources()->pTMapLanguageOffsets->unknown_18;

    TMAPLANGDATA_READ(readOffset, &numValues, 2); // numValues = 12
    for (unsigned short i = 0; i < numValues; i++)
    {
        // index = 1, 2, ..., 12
        TMAPLANGDATA_READ(readOffset, &index, 1);
        // values seem to be: 5,5,3,2,3,3,2,2,3,2,2,2
        TMAPLANGDATA_READ(readOffset, &value, 1);
        // all zero
        TMAPLANGDATA_READ(readOffset, &chance, 1);

        if (rand() % 100 < (int)chance)
        {
            unknown_66_ = value;
            unknown_4f_ = index;
            return;
        }
    }
}

// USA: func_020a4738
// JPN: func_020a64d0
void DetailedTreasureMapData::RegularMapData::GenerateEnviron()
{
    if (GameState::GetInstance()->GetTreasureMapLanguageData() == NULL)
        return;

    unsigned short numEntries = 0;
    unsigned char readEnviron = 0;
    unsigned char readChance = 0;

    int readOffset = GetGameResources()->pTMapLanguageOffsets->environs;

    unsigned char percentile = 0;

    unsigned char rngValue = rand() % 100;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readEnviron, 1);
        TMAPLANGDATA_READ(readOffset, &readChance, 1);

        if (rngValue < readChance + percentile)
        {
            this->environ_ = readEnviron;
            return;
        }

        percentile += readChance;
    }
}

// USA: func_020a4824
// JPN: func_020a65bc
void DetailedTreasureMapData::RegularMapData::GenerateFloorCount()
{
    if (GameState::GetInstance()->GetTreasureMapLanguageData() == NULL)
        return;

    unsigned short numEntries = 0;
    unsigned char readMinQuality = 0;
    unsigned char readMaxQuality = 0;
    unsigned char readMinFloors = 0;
    unsigned char readMaxFloors = 0;

    int readOffset = GetGameResources()->pTMapLanguageOffsets->floorRangesByQuality;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readMinQuality, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxQuality, 1);
        TMAPLANGDATA_READ(readOffset, &readMinFloors, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxFloors, 1);

        if (readMinQuality <= quality_ && quality_ <= readMaxQuality)
        {
            floorCount_ = RandATRangeModular(readMinFloors, readMaxFloors);
            return;
        }
    }
}

// USA: func_020a495c
// JPN: func_020a66f4
void DetailedTreasureMapData::RegularMapData::GenerateMonsterRank()
{
    if (GameState::GetInstance()->GetTreasureMapLanguageData() == NULL)
        return;

    unsigned short numEntries = 0;
    unsigned char readMinQuality = 0;
    unsigned char readMaxQuality = 0;
    unsigned char readMinRank = 0;
    unsigned char readMaxRank = 0;

    int readOffset = GetGameResources()->pTMapLanguageOffsets->startingMonsterRanksByQuality;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readMinQuality, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxQuality, 1);
        TMAPLANGDATA_READ(readOffset, &readMinRank, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxRank, 1);

        if (readMinQuality <= quality_ && quality_ <= readMaxQuality)
        {
            startingMonsterRank_ = RandATRangeModular(readMinRank, readMaxRank);
            return;
        }
    }
}

// JPN: func_020a682c
void DetailedTreasureMapData::RegularMapData::GenerateBoss()
{
    if (GameState::GetInstance()->GetTreasureMapLanguageData() == NULL)
        return;

    unsigned short numEntries = 0;
    unsigned char readMinQuality = 0;
    unsigned char readMaxQuality = 0;
    unsigned char readMinBoss = 0;
    unsigned char readMaxBoss = 0;

    TreasureMapLanguageDataOffsets* offsets = GetGameResources()->pTMapLanguageOffsets;
    int readOffset = offsets->bossRangesByQuality;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readMinQuality, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxQuality, 1);
        TMAPLANGDATA_READ(readOffset, &readMinBoss, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxBoss, 1);

        if (readMinQuality > quality_ || quality_ > readMaxQuality)
            continue;

        int weightReadOffset;
        unsigned short bossTotalWeight = 0;
        unsigned char readBossNumber = 0;
        unsigned short readMonsterID = 0;
        unsigned char readBossWeight = 0;

        for (unsigned short i = readMinBoss; i <= readMaxBoss; i++)
        {
            weightReadOffset = (i - 1) * 4 + 2 + offsets->bossIDsAndWeights;
            TMAPLANGDATA_READ(weightReadOffset, &readBossNumber, 1);
            TMAPLANGDATA_READ(weightReadOffset, &readMonsterID, 2);
            TMAPLANGDATA_READ(weightReadOffset, &readBossWeight, 1);
            bossTotalWeight += readBossWeight;
        }

        unsigned int rng = rand() % bossTotalWeight;

        unsigned short weightAccumulator = 0;
        for (unsigned short i = readMinBoss; i <= readMaxBoss; i++)
        {
            weightReadOffset = 4 * (i - 1) + 2 + offsets->bossIDsAndWeights;
            TMAPLANGDATA_READ(weightReadOffset, &readBossNumber, 1);
            TMAPLANGDATA_READ(weightReadOffset, &readMonsterID, 2);
            TMAPLANGDATA_READ(weightReadOffset, &readBossWeight, 1);

            if (rng < readBossWeight + weightAccumulator)
            {
                bossID_ = i;
                bossMonsterID_ = readMonsterID;
                return;
            }

            weightAccumulator += readBossWeight;
        }
    }
}

// USA: func_020a4d08
// JPN: func_020a6aa0
void DetailedTreasureMapData::RegularMapData::GenerateUnusedChestRanks()
{
    if (GameState::GetInstance()->GetTreasureMapLanguageData() == NULL)
        return;

    unsigned short numEntries = 0;
    int readOffset = GetGameResources()->pTMapLanguageOffsets->seeminglyChestRanksByMonsterRank;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);

    if (numEntries != 12)
        return;

    for (int i = 0; i < 12; i++)
    {
        unsigned char readMonsterRank = 0;
        unsigned char readMinChestRank = 0;
        unsigned char readMaxChestRank = 0;

        TMAPLANGDATA_READ(readOffset, &readMonsterRank, 1);
        TMAPLANGDATA_READ(readOffset, &readMinChestRank, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxChestRank, 1);

        maybeUnusedChestRanks_[i] = RandATRangeModular(readMinChestRank, readMaxChestRank);
    }
}

// USA: func_020a4e08
// JPN: func_020a5ba0
void DetailedTreasureMapData::RegularMapData::GeneratePrefix()
{
    if (GameState::GetInstance()->GetTreasureMapLanguageData() == NULL)
        return;

    unsigned short numEntries = 0;
    unsigned char readMinMonsterRank = 0;
    unsigned char readMaxMonsterRank = 0;
    unsigned char readMinPrefixIdx = 0;
    unsigned char readMaxPrefixIdx = 0;

    int readOffset = GetGameResources()->pTMapLanguageOffsets->prefixRangesByMonsterRank;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readMinMonsterRank, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxMonsterRank, 1);
        TMAPLANGDATA_READ(readOffset, &readMinPrefixIdx, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxPrefixIdx, 1);

        if (readMinMonsterRank <= startingMonsterRank_ &&
            startingMonsterRank_ <= readMaxMonsterRank)
        {
            prefix_ = RandATRangeModular(readMinPrefixIdx, readMaxPrefixIdx);
            return;
        }
    }
}

// USA: func_020a4f40
// JPN: func_020a6cd8
void DetailedTreasureMapData::RegularMapData::GenerateSuffix()
{
    if (GameState::GetInstance()->GetTreasureMapLanguageData() == NULL)
        return;

    unsigned short numEntries = 0;
    unsigned char readMinBossIdx = 0;
    unsigned char readMaxBossIdx = 0;
    unsigned char readMinSuffixIdx = 0;
    unsigned char readMaxSuffixIdx = 0;

    int readOffset = GetGameResources()->pTMapLanguageOffsets->suffixRangesByBoss;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readMinBossIdx, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxBossIdx, 1);
        TMAPLANGDATA_READ(readOffset, &readMinSuffixIdx, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxSuffixIdx, 1);

        if (readMinBossIdx <= bossID_ &&
            bossID_ <= readMaxBossIdx)
        {
            suffix_ = RandATRangeModular(readMinSuffixIdx, readMaxSuffixIdx);
            return;
        }
    }
}

// USA: func_020a5078
// JPN: func_020a6e10
void DetailedTreasureMapData::RegularMapData::GenerateLocaleRank()
{
    if (GameState::GetInstance()->GetTreasureMapLanguageData() == NULL)
        return;

    unsigned short numEntries = 0;
    unsigned char readMinFloorCount = 0;
    unsigned char readMaxFloorCount = 0;
    unsigned char readMinLocaleRank = 0;
    unsigned char readMaxLocaleRank = 0;

    int readOffset = GetGameResources()->pTMapLanguageOffsets->localeRankRangesByFloorCount;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readMinFloorCount, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxFloorCount, 1);
        TMAPLANGDATA_READ(readOffset, &readMinLocaleRank, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxLocaleRank, 1);

        if (readMinFloorCount <= floorCount_ &&
            floorCount_ <= readMaxFloorCount)
        {
            localeRank_ = RandATRangeModular(readMinLocaleRank, readMaxLocaleRank);
            return;
        }
    }
}

#if defined(usa)

// USA: func_020a51b0
void DetailedTreasureMapData::RegularMapData::GenerateNameBuffers()
{    
    if (prefix_ == 0 || suffix_ == 0 || localeRank_ == 0 || level_ == 0)
        return;

    if (GameState::GetInstance()->GetTreasureMapLanguageData() == NULL)
        return;

    nameNoLevel_[0] = '\0';

    // quantities read repeatedly from the binary file
    unsigned short numEntries = 0;
    unsigned char readIndex = 0;
    unsigned short stringUnknown = 0;
    unsigned short stringLength = 0;

    // Important for register nonsense to declare these here
    int nameIdx;
    int readOffset;
    
    unsigned char* langData = GameState::GetInstance()->GetTreasureMapLanguageData();
    int* offsetArray = (int*)(GetGameResources()->pTMapLanguageOffsets);   

    // choose the order of the words based on the language
    unsigned char partOrder[3]; 
    switch (func_0200fb08(GameState::GetInstance()))
    {
    // English & German
    // e.g. Granite (0) Tunnel (2) of Woe (1)
    case 1:
    case 3:
        partOrder[0] = 0;
        partOrder[1] = 2;
        partOrder[2] = 1;
        break;
    // French, Italian, Spanish
    // e.g. Tunnel (2) of Granite (0) of Woe (1)
    case 2:
    case 4:
    case 5:
        partOrder[0] = 2;
        partOrder[1] = 0;
        partOrder[2] = 1;
        break;
    case 0:
    default:
        partOrder[0] = 0;
        partOrder[1] = 2;
        partOrder[2] = 1;
        break;
    }
    
    unsigned char indices[3];
    indices[0] = prefix_;
    indices[1] = suffix_;
    indices[2] = localeRank_;

    char tempBuffer[256];
    
    for (int i = 0; i < 3; i++)
    {
        static const unsigned char ptrListIndexLookup[] = { 4, 13, 11 };

        int currentNamePart = partOrder[i];
        int ptrListIndex = ptrListIndexLookup[currentNamePart];
        
        readOffset = offsetArray[ptrListIndex];
        
        TMAPLANGDATA_READ(readOffset, &numEntries, 2);
         
        for (unsigned short entryLoop = 0; entryLoop < numEntries; entryLoop++)
        {
            nameIdx = indices[currentNamePart];
            if (ptrListIndex == 11)
            {
                TMAPLANGDATA_READ(readOffset, &readIndex, 1);
                for (unsigned short loopEnviron = 1; loopEnviron <= 5; loopEnviron++)
                {
                    TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
                    TMAPLANGDATA_READ(readOffset, &stringLength, 2);
                    if (environ_ == loopEnviron && nameIdx == readIndex)
                    {
                        VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
                        tempBuffer[stringLength] = '\0';
                        strcat(nameNoLevel_, tempBuffer);
                        entryLoop = numEntries; // hack to escape the j-level loop
                        break;
                    }
                    readOffset += stringLength;
                }
            }
            else
            {
                TMAPLANGDATA_READ(readOffset, &readIndex, 1);
                TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
                TMAPLANGDATA_READ(readOffset, &stringLength, 2);
                if (nameIdx == readIndex)
                {
                    VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
                    tempBuffer[stringLength] = '\0';
                    strcat(nameNoLevel_, tempBuffer);
                    break;
                }
                readOffset += stringLength;
            }
        }
    }

    sprintf(levelString_, "%s%d", GetGameString(GameString_Lv), level_);
    sprintf(topScreenName_, "%s %s", nameNoLevel_, levelString_);
}

// USA: func_020a54d0
void DetailedTreasureMapData::RegularMapData::GeneratePopupName()
{    
    if (prefix_ == 0 || suffix_ == 0 || localeRank_ == 0 || level_ == 0)
        return;

    if (GameState::GetInstance()->GetTreasureMapLanguageData() == NULL)
        return;

    popupName_[0] = '\0';

    // quantities read repeatedly from the binary file
    unsigned short numEntries = 0;
    unsigned char readIndex = 0;
    unsigned short stringUnknown = 0;
    unsigned short stringLength = 0;

    // Important for register nonsense to declare these here
    int nameIdx;
    int readOffset;
    
    unsigned char* langData = GameState::GetInstance()->GetTreasureMapLanguageData();
    int* offsetArray = (int*)(GetGameResources()->pTMapLanguageOffsets);   

    // choose the order of the words based on the language
    unsigned char partOrder[3]; 
    switch (func_0200fb08(GameState::GetInstance()))
    {
    // English & German
    // e.g. Granite (0) Tunnel (2) of Woe (1)
    case 1:
    case 3:
        partOrder[0] = 0;
        partOrder[1] = 2;
        partOrder[2] = 1;
        break;
    // French, Italian, Spanish
    // e.g. Tunnel (2) of Granite (0) of Woe (1)
    case 2:
    case 4:
    case 5:
        partOrder[0] = 2;
        partOrder[1] = 0;
        partOrder[2] = 1;
        break;
    case 0:
    default:
        partOrder[0] = 0;
        partOrder[1] = 2;
        partOrder[2] = 1;
        break;
    }
    
    unsigned char indices[3];
    indices[0] = prefix_;
    indices[1] = suffix_;
    indices[2] = localeRank_;

    char tempBuffer[64];
    
    for (int i = 0; i < 3; i++)
    {
        static const unsigned char ptrListIndexLookup[] = { 4, 13, 11 };

        int currentNamePart = partOrder[i];
        int ptrListIndex = ptrListIndexLookup[currentNamePart];
        
        readOffset = offsetArray[ptrListIndex];
        
        TMAPLANGDATA_READ(readOffset, &numEntries, 2);
         
        for (unsigned short entryLoop = 0; entryLoop < numEntries; entryLoop++)
        {
            nameIdx = indices[currentNamePart];
            if (ptrListIndex == 11) // locale
            {
                TMAPLANGDATA_READ(readOffset, &readIndex, 1);
                for (unsigned short loopEnviron = 1; loopEnviron <= 5; loopEnviron++)
                {
                    TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
                    TMAPLANGDATA_READ(readOffset, &stringLength, 2);
                    if (environ_ == loopEnviron && nameIdx == readIndex)
                    {
                        VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
                        tempBuffer[stringLength] = '\0';
                        strcat(popupName_, tempBuffer);
                        entryLoop = numEntries; // hack to escape the j-level loop
                        break;
                    }
                    readOffset += stringLength;
                }
            }
            else
            {
                TMAPLANGDATA_READ(readOffset, &readIndex, 1);
                TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
                TMAPLANGDATA_READ(readOffset, &stringLength, 2);
                if (nameIdx == readIndex)
                {
                    VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
                    tempBuffer[stringLength] = '\0';
                    strcat(popupName_, tempBuffer);
                    break;
                }
                readOffset += stringLength;
            }
        }
    }

    sprintf(tempBuffer, " %s%d", GetGameString(GameString_Lv), level_);
    strcat(popupName_, tempBuffer);
}

#elif defined(jpn)

// JPN: func_020a6f48
void DetailedTreasureMapData::RegularMapData::GenerateNameBuffers()
{    
    if (prefix_ == 0 || suffix_ == 0 || localeRank_ == 0 || level_ == 0)
        return;

    if (GameState::GetInstance()->GetTreasureMapLanguageData() == NULL)
        return;

    nameNoLevel_[0] = '\0';

    int readOffset;
    
    // quantities read repeatedly from the binary file
    unsigned short numEntries = 0;
    unsigned char readIndex = 0;
    unsigned short stringUnknown = 0;
    unsigned short stringLength = 0;
    char tempBuffer[256];
    
    unsigned char* langData = GameState::GetInstance()->GetTreasureMapLanguageData();
    TreasureMapLanguageDataOffsets* offsetArray = GetGameResources()->pTMapLanguageOffsets;
    
    readOffset = offsetArray->prefixNames;
    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readIndex, 1);
        TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
        TMAPLANGDATA_READ(readOffset, &stringLength, 2);
        if (prefix_ == readIndex)
        {
            VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
            tempBuffer[stringLength] = '\0';
            strcat(nameNoLevel_, tempBuffer);
            strcpy(prefixString_, tempBuffer);
            break;
        }
        readOffset += stringLength;
    }

    readOffset = offsetArray->suffixNames;
    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readIndex, 1);
        TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
        TMAPLANGDATA_READ(readOffset, &stringLength, 2);
        if (suffix_ == readIndex)
        {
            VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
            tempBuffer[stringLength] = '\0';
            strcat(nameNoLevel_, tempBuffer);
            strcpy(suffixString_, tempBuffer);
            break;
        }
        readOffset += stringLength;
    }

    char nameUndecorated[256];
    RemoveFurigana(nameNoLevel_, nameUndecorated);
    strcat(nameUndecorated, "’n}");
    strcpy(nameNoLevel_, nameUndecorated);
    sprintf(levelString_, JPSTRING_LV "%d", level_);
    sprintf(topScreenName_, "%s<W=3>%s", nameNoLevel_, levelString_);
}

// JPN: func_020a71f8
void DetailedTreasureMapData::RegularMapData::GeneratePopupName()
{    
    if (prefix_ == 0 || suffix_ == 0 || localeRank_ == 0 || level_ == 0)
        return;

    if (GameState::GetInstance()->GetTreasureMapLanguageData() == NULL)
        return;

    popupName_[0] = '\0';

    int readOffset;
    
    // quantities read repeatedly from the binary file
    unsigned short numEntries = 0;
    unsigned char readIndex = 0;
    unsigned short stringUnknown = 0;
    unsigned short stringLength = 0;
    char tempBuffer[64];
    
    unsigned char* langData = GameState::GetInstance()->GetTreasureMapLanguageData();
    TreasureMapLanguageDataOffsets* offsetArray = GetGameResources()->pTMapLanguageOffsets;

    readOffset = offsetArray->prefixNames;
    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readIndex, 1);
        TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
        TMAPLANGDATA_READ(readOffset, &stringLength, 2);
        if (prefix_ == readIndex)
        {
            VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
            tempBuffer[stringLength] = '\0';
            strcat(popupName_, tempBuffer);
            strcpy(prefixString_, tempBuffer);
            break;
        }
        readOffset += stringLength;
    }

    readOffset = offsetArray->suffixNames;
    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readIndex, 1);
        TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
        TMAPLANGDATA_READ(readOffset, &stringLength, 2);
        if (suffix_ == readIndex)
        {
            VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
            tempBuffer[stringLength] = '\0';
            strcat(popupName_, tempBuffer);
            strcpy(suffixString_, tempBuffer);
            break;
        }
        readOffset += stringLength;
    }

    readOffset = offsetArray->localeNames;
    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readIndex, 1);
        for (unsigned short loopEnviron = 1; loopEnviron <= 5; loopEnviron++)
        {
            TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
            TMAPLANGDATA_READ(readOffset, &stringLength, 2);
            if (environ_ == loopEnviron && localeRank_ == readIndex)
            {
                VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
                tempBuffer[stringLength] = '\0';
                strcat(popupName_, tempBuffer);
                strcpy(localeString_, tempBuffer);
                i = numEntries; // hack to escape the outer loop
                break;
            }
            readOffset += stringLength;
        }
    }

    sprintf(tempBuffer, JPSTRING_LV "%d", level_);
    strcat(popupName_, tempBuffer);
}

#endif