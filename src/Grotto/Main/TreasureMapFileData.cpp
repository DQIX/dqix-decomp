#include "Grotto/Main/DetailedTreasureMapData.h"
#include "Filesystem/BackgroundLoader.h"
#include "Filesystem/FileIO.h"
#include "NitroSDK/Memory.h"
#include "GameState/GameState.h"

#define BINARY_READ_AND_ADVANCE(buffer, offset, dst, len) \
    (VectorizedInvertedMemcpy((buffer) + (offset), (dst), (len)), offset += (len))

#define TMAPLANGDATA_READ(offset, into, len) \
    BINARY_READ_AND_ADVANCE(GameState::GetInstance()->GetTreasureMapLanguageData(), offset, into, len)

void DetailedTreasureMapData::LoadLegacyBossStats(bool compute, const unsigned char* providedArchive)
{
    if (mapType_ != TreasureMapType_Legacy)
        return;

    if (!compute)
    {
        legacy_.stats_.dropListIndex = 0;
        legacy_.stats_.newDropListAtNextLevel = false;
        return;
    }
    
    BackgroundLoader::AddLockGlobal();
    unsigned int archiveSize = 0;
    const unsigned char* usedArchive = g_fileScratchSpace;
    
    if (providedArchive)
        usedArchive = providedArchive;
    else
    {
        if (!LoadFileIntoMemory("data/tmap/param.pac", const_cast<unsigned char*>(usedArchive), &archiveSize))
        {
            BackgroundLoader::RemoveLockGlobal();
            return;
        }
    }

    char innerFileName[256];
    sprintf(innerFileName, "p_%d.dat", legacy_.bossMonsterID_);
    const unsigned char* innerFileData;
    unsigned int innerFileSize;
    if (!GetFileInNarc(usedArchive, innerFileName, reinterpret_cast<const void**>(&innerFileData), &innerFileSize, 0))
    {
        BackgroundLoader::RemoveLockGlobal();
        return;
    }

    BackgroundLoader::RemoveLockGlobal();

    unsigned int stride = 0x18;
    unsigned int offset = stride * (legacy_.level_ - 1);
    
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy_.stats_.maxHP, 4);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy_.stats_.maxMP, 4);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy_.stats_.agility, 2);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy_.stats_.attack, 2);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy_.stats_.defense, 2);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy_.stats_.alternateVersion, 1);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy_.stats_.rewardExp, 4);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy_.stats_.rewardGold, 4);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy_.stats_.dropListIndex, 1);
    
    legacy_.stats_.newDropListAtNextLevel = false;
    if (legacy_.level_ < 99)
    {
        LegacyBossStats next;
        offset = stride * legacy_.level_;
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.maxHP, 4);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.maxMP, 4);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.agility, 2);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.attack, 2);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.defense, 2);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.alternateVersion, 1);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.rewardExp, 4);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.rewardGold, 4);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.dropListIndex, 1);
        
        if (legacy_.stats_.dropListIndex < next.dropListIndex)
            legacy_.stats_.newDropListAtNextLevel = true;
    }

    offset = stride * 99;
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy_.stats_.numLevelUpMoves, 1);
    for (int i = 0; i < legacy_.stats_.numLevelUpMoves; i++)
    {
        LegacyBossStats::LevelUpMove* dst = &legacy_.stats_.levelUpMoves[i];
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &dst->moveID, 2);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &dst->level, 1);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &dst->announceLearn, 1);
    }
}

void DetailedTreasureMapData::LoadTreasures()
{
    if (GameState::GetInstance()->GetTreasureMapLanguageData() == NULL)
        return;

    TreasureMapLanguageDataOffsets* offsets = GetGameResources()->pTMapLanguageOffsets;
    unsigned int offset;
    bool foundBoss = false;

    if (mapType_ == TreasureMapType_Regular)
    {
        if (treasureItemIDs_[0] != 0 && treasureItemIDs_[1] != 0 && treasureItemIDs_[2] != 0)
            return;
        offset = offsets->grottoBossDrops;
        do
        {
            unsigned char bossIndex;
            unsigned short regularID;
            unsigned int innerOffset = offset;
            TMAPLANGDATA_READ(innerOffset, &bossIndex, 1);
            TMAPLANGDATA_READ(innerOffset, &regularID, 2);
            if (regularID == regular_.bossMonsterID_)
            {
                rand();
                foundBoss = true;
                offset = innerOffset;
                break;
            }
            else
                offset += 12;
        } while (offset < offsets->legacyBossDrops);
    }
    else
    {
        offset = offsets->legacyBossDrops;
        do
        {
            unsigned short legacyID;
            TMAPLANGDATA_READ(offset, &legacyID, 2);
            if (legacyID == legacy_.bossMonsterID_)
            {
                foundBoss = true;
                offset += (legacy_.stats_.dropListIndex - 1) * 9;
                break;
            }
            else
                offset += 90;
        } while (offset < offsets->legacyBossData);
    }

    if (foundBoss)
    {
        for (int i = 0; i < 3; i++)
        {
            TMAPLANGDATA_READ(offset, &treasureItemIDs_[i], 2);
            TMAPLANGDATA_READ(offset, &treasureDropRates_[i], 1);
        }
    }
}