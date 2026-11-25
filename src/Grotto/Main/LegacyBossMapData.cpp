#include "Grotto/Main/DetailedTreasureMapData.h"
#include "GameState/GameState.h"
#include "NitroSDK/Memory.h"
#include "Resource/GameStrings.h"

#define BINARY_READ_AND_ADVANCE(buffer, offset, dst, len) \
    (VectorizedInvertedMemcpy((buffer) + (offset), (dst), (len)), offset += (len))

#define TMAPLANGDATA_READ(offset, into, len) \
    BINARY_READ_AND_ADVANCE(GameState::GetInstance()->GetTreasureMapLanguageData(), offset, into, len)

// USA: func_020a425c
void DetailedTreasureMapData::LegacyBossMapData::Populate(
    unsigned char newBossID, unsigned char newLevel, unsigned short newMinTurns)
{
    if (GameState::GetInstance()->GetTreasureMapLanguageData() == 0)
        return;

    bossMonsterID_ = 0;
    bossID_ = newBossID;

    for (int i = 0; i < 3; i++)
        alternateVersionIDs_[i] = 0;

    unsigned short numEntries = 0;

    unsigned char readBossID = 0;
    unsigned short readMonsterID = 0;
    unsigned short readAlternates[3];
    unsigned short readUnknown = 0; // seems to precede every string and always = 8
    unsigned short readStringLen = 0;

    TreasureMapLanguageDataOffsets* langData = GetGameResources()->pTMapLanguageOffsets;
    unsigned char* dataPtr = GameState::GetInstance()->GetTreasureMapLanguageData();

    int readOffset = langData->legacyBossData;
    
    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (int i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readBossID, 2);
        TMAPLANGDATA_READ(readOffset, &readMonsterID, 2);
        for (int j = 0; j < 3; j++)
        {
            TMAPLANGDATA_READ(readOffset, &readAlternates[j], 2);
        }

        TMAPLANGDATA_READ(readOffset, &readUnknown, 2);
        TMAPLANGDATA_READ(readOffset, &readStringLen, 2);

        if (newBossID == readBossID)
        {
            bossMonsterID_ = readMonsterID;
            for (int j = 0; j < 3; j++)
                alternateVersionIDs_[j] = readAlternates[j];
            VectorizedInvertedMemcpy(dataPtr + readOffset, bossName_, readStringLen);
            bossName_[readStringLen] = '\0';
#if defined(jpn)
            break;
#endif
        }

        readOffset += readStringLen;
// in international versions, the binary file also stores strings such as
// "Baramos's Map", presumably because grammatical differences between multiple
// languages would be painful to implement in code
#if defined(usa) 
        TMAPLANGDATA_READ(readOffset, &readUnknown, 2);
        TMAPLANGDATA_READ(readOffset, &readStringLen, 2);

        if (newBossID == readBossID)
        {
            VectorizedInvertedMemcpy(dataPtr + readOffset, mapNameNoLevel_, readStringLen);
            mapNameNoLevel_[readStringLen] = '\0';
            break;
        }
        readOffset += readStringLen;
#endif
    }

    if (bossMonsterID_ == 0)
        return;

    level_ = newLevel;
    minTurns_ = newMinTurns;

#if defined(usa)
    sprintf(mapNameNoLevel_v2_, "%s", mapNameNoLevel_);
    strcpy(seeminglyEmptyBuffer_, "");
    sprintf(mapLevelString_, "%s%d", GetGameString(GameString_Lv), level_);
    sprintf(topScreenName_, "%s %s", mapNameNoLevel_, mapLevelString_);
    sprintf(popupName_, "%s %s", bossName_, mapLevelString_);
#elif defined(jpn)
    char bossNameUndecorated[256];

    RemoveFurigana(bossName_, bossNameUndecorated);
    sprintf(mapNameNoLevel_, "%s‚Ì’n}", bossNameUndecorated);
    sprintf(bossNameGenitive_, "%s‚Ì", bossName_);
    strcpy(fixedStringChizu_, "[’n}/‚¿‚¸]");
    strcpy(mapLevelString_, "");
    sprintf(mapLevelString_, JPSTRING_LV "%d", level_);
    sprintf(topScreenName_, "%s<W=3>%s", mapNameNoLevel_, mapLevelString_);
    sprintf(popupName_, "%s" JPSTRING_LV "%d‚Ì[ŠÔ/‚Ü]", bossName_, level_);
#endif
}

// USA: func_020a451c
// JPN: func_020a62c8
void DetailedTreasureMapData::LegacyBossMapData::WriteMapLevelString()
{
#if defined(usa)
    const char* lvlPrefix = GetGameString(GameString_Lv);
    sprintf(mapLevelString_, "%s%d", lvlPrefix, level_);
#elif defined(jpn)
    sprintf(mapLevelString_, JPSTRING_LV "%d", level_);
#endif
}

// USA: func_020a454c
// JPN: func_020a62e4
bool DetailedTreasureMapData::LegacyBossMapData::CanUseLevelUpMove(unsigned short id)
{
    for (int i = 0; i < stats_.numLevelUpMoves; i++)
    {
        LegacyBossStats::LevelUpMove* move = &stats_.levelUpMoves[i];
        if (move->moveID == id)
            return move->level <= this->level_;
    }

    // If not a level up move, treat as having it by default
    return true;
}

// USA: func_020a45a0
// JPN: func_020a6338
unsigned short DetailedTreasureMapData::LegacyBossMapData::GetLearnedMove(unsigned char atLevel, int filter)
{
    for (int i = 0; i < stats_.numLevelUpMoves; i++)
    {
        LegacyBossStats::LevelUpMove* move = &stats_.levelUpMoves[i];
        if (move->level != atLevel)
            continue;

        if (filter == 0)
        {
            if (move->announceLearn != false)
                continue;
            return move->moveID;
        }
        else if (filter == 1)
        {
            if (move->announceLearn != true)
                continue;
            return move->moveID;
        }
        return move->moveID;
    }

    return 0;
}