#include "Grotto/Main/TreasureMapDataStructs.h"
#include "GameState/GameState.h"
#include "Grotto/Main/GrottoStruct.h"
#include "Resource/GameResources.h"
#include "Filesystem/FileIO.h"
#include "Filesystem/BackgroundLoader.h"
#include "NitroSDK/Memory.h"
#include "Resource/GameStrings.h"
#include "std_library_functions.h"
#include <asmhacks.h>

#ifdef jpn
#define func_020a1df8 func_020a3b70
#define func_020a1e54 func_020a3bcc
#endif

extern "C"
{
    void func_020a1df8(unsigned int);
    void func_020a1e54(unsigned int);

    // copies character name into the buffer? (not used in jpn version)
    void func_020426bc(void*, char* buffer, int);
}

#ifdef jpn

// JPN: func_020a5c20
void RemoveFurigana(const char* src, char* dst)
{  
    if (dst == NULL)
        return;

    VectorizedMemset(dst, 0, 4);

    unsigned int mode = 0;
    int srcIndex;
    int dstIndex = 0;
    
    for (srcIndex = 0; src[srcIndex] != '\0'; srcIndex++)
    {
        int inChar = src[srcIndex];

        if (inChar == '[')
        {
            mode = 1;
            continue;
        }
        
        if (inChar == '/')
        {
            mode = 2;
            continue;
        }
        
        if (inChar == ']')
        {
            mode = 0;
            continue;
        }
        
        if (mode <= 1)
        {
            unsigned char castChar = inChar;
            bool twoBytes;
            if ((castChar >= 0x81 && castChar <= 0x9F) || (castChar >= 0xE0 && castChar <= 0xFC))
                twoBytes = true;
            else
                twoBytes = false;

            if (twoBytes)
            {
                // totally normal code, nothing to see here
                // (we need it to force some register placements)
                int srcIndexPlusOne = srcIndex + 1;
                dst[dstIndex] = src[srcIndexPlusOne - 1];
                srcIndex++; // note srcIndex = srcIndexPlusOne doesn't work
                dstIndex++;
            }

            dst[dstIndex] = src[srcIndex];
            dstIndex++;
        }
    }
    
    dst[dstIndex] = 0;
}

#endif

// USA: func_020a3ee4
// JPN: func_020a5cfc
bool IsMonsterIDLegacyBoss(unsigned short id)
{
    // v1 form of Dragonlord (500) through Mortamor (508)
    if (id >= 500 && id <= 508)
        return true;

    // v1 form of Orgodemir, Dhoulmagus, Rhapthorne, Nokturnus
    if (id >= 511 && id <= 514)
        return true;

    // alternate patterns for all bosses except Nokturnus
    if (id >= 600 && id <= 660)
        return true;

    // alternate patterns for Nokturnus
    if (id >= 769 && id <= 775)
        return true;

    return false;
}

struct MapTypeEntry
{
    unsigned short itemID;
    unsigned char legacyBossID;
    bool isLegacy;
};

// USA: func_020a3f54
// JPN: func_020a5d6c
bool GetTreasureMapTypeFromItemID(unsigned short itemID, unsigned char* out)
{
    if (out == NULL)
        return false;

    static const MapTypeEntry treasureMapItemIDs[] = {
        { 1000, 0, false },  // regular map
        { 1001, 1, true },   // Dragonlord
        { 1002, 2, true },   // Malroth
        { 1003, 3, true },   // Baramos
        { 1004, 4, true },   // Zoma
        { 1005, 5, true },   // Psaro (comes before Estark here)
        { 1006, 6, true },   // Estark
        { 1007, 7, true },   // Nimzo
        { 1008, 8, true },   // Murdaw
        { 1009, 9, true },   // Mortamor
        { 1010, 10, true },  // Nokturnus (this time he's not at the end)
        { 1011, 11, true },  // Orgodemir
        { 1012, 12, true },  // Dhoulmagus
        { 1013, 13, true },  // Rhapthorne
        { 0 }
    };

    for (int i = 0; treasureMapItemIDs[i].itemID != 0; i++)
    {
        const MapTypeEntry* entry = &treasureMapItemIDs[i];
        
        unsigned short thisID = entry->itemID;
        if (thisID == 0 || itemID != thisID)
            continue;
        
        unsigned char mapType;
        unsigned char thisLegacyBossID;
        unsigned short isLegacy;

        if (thisID == 1000)
        {
            isLegacy = entry->isLegacy;
            thisLegacyBossID = entry->legacyBossID;
            mapType = TreasureMapType_Regular;
        }
        else
        {
            isLegacy = entry->isLegacy;
            thisLegacyBossID = entry->legacyBossID;
            mapType = TreasureMapType_Legacy;
        }

        out[0] = mapType;
        out[1] = thisLegacyBossID;
        *(unsigned short*)(out + 2) = isLegacy;
        
        return true;
    }
    
    return false;
}

void DetailedTreasureMapData::RegularMapData::Populate(unsigned short newseed, unsigned char newquality)
{
    VectorizedMemset(this, 0, sizeof(RegularMapData));
    seed_ = newseed;
    srand(seed_);
    quality_ = newquality;
    func_020a1df8(4);

    GenerateUnknownData();
    GenerateEnviron();
    GenerateFloorCount();
    GenerateMonsterRank();
    GenerateBoss();
    GenerateUnusedChestRanks();
    GeneratePrefix();
    GenerateSuffix();
    GenerateLocaleRank();

    int levelSum = floorCount_ + startingMonsterRank_ + bossID_;
    int unclampedLevel = (rand() % 11) - 5 + (levelSum - 4) * 3;
    if (unclampedLevel <= 0)
        level_ = 1;
    else if (unclampedLevel > 99)
        level_ = 99;
    else
        level_ = unclampedLevel;

    GenerateNameBuffers();
    GeneratePopupName();
    
    func_020a1e54(1);
}

unsigned short DetailedTreasureMapData::LegacyBossMapData::MaybeGetCurrentAlternateID() const
{
    int idx = stats_.alternateVersion;
    if (idx >= 1 && idx <= 3)
        return alternateVersionIDs_[idx - 1];
    return 0;
}

void DetailedTreasureMapData::Clear()
{
    VectorizedMemset(this, 0, sizeof(DetailedTreasureMapData));
}

void DetailedTreasureMapData::BlankFunction() const {}

bool DetailedTreasureMapData::UpdateFollowingCompletion(bool levelledUp, unsigned short numTurns)
{
    if (mapType_ != TreasureMapType_Legacy)
        return false;

    // Retrieving the name of the player character I guess?
#ifndef jpn
    // Based on how the jpn version works, I would guess this is undoing the
    // custom text encoding (e.g. lowercase a is 0x2A vs ascii 0x61)
    void* playerRelatedPtr = *(void**)((intptr_t)GameState::GetInstance()->GetProtagonist() + 0x134);
    char asciiName[10] = { 0 };
    func_020426bc(playerRelatedPtr, asciiName, 1);
#else
    // 0200fc28 is the address in the japanese version
    char* asciiName = *(char**)((intptr_t)GameState::GetInstance()->GetProtagonist() + 0x134);
#endif

    discoveryState_ = DiscoveryState_Cleared;

    VectorizedMemset(clearedBy_, 0, 12);
    VectorizedInvertedMemcpy(asciiName, clearedBy_, 10);

    if (levelledUp && legacy_.stats_.newDropListAtNextLevel)
    {
        discoveredTreasures_[0] = true;
        discoveredTreasures_[1] = false;
        discoveredTreasures_[2] = false;
    }

    if (levelledUp)
    {
        legacy_.level_++;
        if (legacy_.level_ > 99)
            legacy_.level_ = 99;
        legacy_.minTurns_ = 0;   
    }
    else
    {
        if (numTurns < 1000 && (legacy_.minTurns_ == 0 || numTurns < legacy_.minTurns_))
        {
            legacy_.minTurns_ = numTurns;
            return true;
        }
    }

    legacy_.WriteMapLevelString();
    GrottoStruct* grotto = GameState::GetInstance()->GetGrottoStruct();
    strcpy(grotto->activeMapNameNoLevel, legacy_.mapNameNoLevel_);
    grotto->activeMapLevel = legacy_.level_;
    return false;
}

unsigned int DetailedTreasureMapData::GetLevel() const
{
    unsigned int ret = 1;
    if (mapType_ == TreasureMapType_Regular)
        ret = regular_.level_;
    else if (mapType_ == TreasureMapType_Legacy)
        ret = legacy_.level_;
    return ret;
}

void GrottoStruct::LoadActiveMetadataFromDetailed(DetailedTreasureMapData* from)
{
    ExportTreasureMapMetadata(from, &activeMapData);
}