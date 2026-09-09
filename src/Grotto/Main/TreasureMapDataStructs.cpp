#include "Grotto/Main/TreasureMapDataStructs.h"
#include "std_library_functions.h"
#include "Grotto/Main/GrottoStruct.h"
#include "Combat/Main/BattleList.h"
#include "System/Memory.h"
#include "Resource/GameResources.h"

#ifdef jpn
#define func_020a1df8 func_020a3b70
#define func_020a57e4 func_020a7588
#define func_020a5b1c func_020a78c0
#define func_020a1e54 func_020a3bcc
#endif

extern "C"
{
    // some relation to loading overlays
    void func_020a1df8(unsigned int);
    void func_020a1e54(int);
}

#define TMAPLANGDATA_READ(offset, into, len) \
    (VectorizedInvertedMemcpy(GetTreasureMapLanguageData(GetBattleStruct()) + (offset), (into), (len)), offset += (len))

bool ExportDetailedTreasureMapData(const TreasureMapMetadata* from,
    DetailedTreasureMapData* to, bool computeLegacyStats, const unsigned char* legacyStatsData)
{
    GameResources* resources = func_ov017_0218b5b0();

    if (from == NULL || to == NULL)
        return false;

    if (from->DiscoveryStateAndMapTypeAndUnknown == 0)
        return false;

    VectorizedMemset(to, 0, sizeof(DetailedTreasureMapData));
    if (from->DiscoveryStateAndMapTypeAndUnknown & 0x01)
        to->discoveryState_ = DiscoveryState_Undiscovered;
    else if (from->DiscoveryStateAndMapTypeAndUnknown & 0x02)
        to->discoveryState_ = DiscoveryState_Discovered;
    else if (from->DiscoveryStateAndMapTypeAndUnknown & 0x04)
        to->discoveryState_ = DiscoveryState_Cleared;

    if (from->DiscoveryStateAndMapTypeAndUnknown & 0x08)
        to->mapType_ = TreasureMapType_Regular;
    else if (from->DiscoveryStateAndMapTypeAndUnknown & 0x10)
        to->mapType_ = TreasureMapType_Legacy;

    VectorizedMemset(to->discoveredBy_, 0, 12);
    VectorizedMemset(to->clearedBy_, 0, 12);

    VectorizedInvertedMemcpy(from->DiscoveredBy, to->discoveredBy_, 10);
    VectorizedInvertedMemcpy(from->ClearedBy, to->clearedBy_, 10);

    if (GetTreasureMapLanguageData(GetBattleStruct()) == 0)
        return false;

    int readOffset = resources->pTMapLanguageOffsets->mapLocations;
    
    unsigned short numEntries;
    TMAPLANGDATA_READ(readOffset, &numEntries, sizeof(numEntries));
    
    for (int i = 1; i <= numEntries; i++)
    {
        unsigned short unused8;
        unsigned short stringLength;
        TMAPLANGDATA_READ(readOffset, &unused8, 2);
        TMAPLANGDATA_READ(readOffset, &stringLength, 2);
        TMAPLANGDATA_READ(readOffset, to->mapImageName_, stringLength);
        to->mapImageName_[stringLength] = '\0';

        TMAPLANGDATA_READ(readOffset, &to->entranceZoneID_, 4);
        TMAPLANGDATA_READ(readOffset, &to->entranceX_, 4);
        TMAPLANGDATA_READ(readOffset, &to->entranceY_, 4);
        TMAPLANGDATA_READ(readOffset, &to->entranceZ_, 4);

        if (i == from->Location)
            break;
    }

    to->mapLocation_ = from->Location;
    if (to->mapType_ == TreasureMapType_Regular)
    {
        VectorizedMemset(&to->regular_, 0, sizeof(to->regular_));
        to->regular_.Populate(from->SeedOrMinTurns, from->QualityOrLegacyBossID);
    }
    else if (to->mapType_ == TreasureMapType_Legacy)
    {
        VectorizedMemset(&to->legacy_, 0, sizeof(to->legacy_));
        to->legacy_.Populate(from->QualityOrLegacyBossID, from->LegacyBossLevel, from->SeedOrMinTurns);
    }

    func_020a1df8(4);
    to->LoadLegacyBossStats(computeLegacyStats, legacyStatsData);
    to->LoadTreasures();
    func_020a1e54(1);

    for (int i = 0; i < 3; i++)
        to->discoveredTreasures_[i] = false;

    if (from->TreasureDiscoveryFlags & 1)
        to->discoveredTreasures_[0] = true;
    if (from->TreasureDiscoveryFlags & 2)
        to->discoveredTreasures_[1] = true;
    if (from->TreasureDiscoveryFlags & 4)
        to->discoveredTreasures_[2] = true;

    for (int i = 0; i < 3; i++)
    {
        if (to->treasureDropRates_[i] == 100 && to->discoveryState_ != DiscoveryState_Undiscovered)
            to->discoveredTreasures_[i] = true;
    }
    
    return true;
}

bool ExportTreasureMapMetadata(const DetailedTreasureMapData* from, TreasureMapMetadata* to)
{
    if (from == NULL || to == NULL)
        return false;

    VectorizedMemset(to, 0, sizeof(TreasureMapMetadata));

    if (from->discoveryState_ == DiscoveryState_Undiscovered)
        to->DiscoveryStateAndMapTypeAndUnknown |= 0x01;
    else if (from->discoveryState_ == DiscoveryState_Discovered)
        to->DiscoveryStateAndMapTypeAndUnknown |= 0x02;
    else if (from->discoveryState_ == DiscoveryState_Cleared)
        to->DiscoveryStateAndMapTypeAndUnknown |= 0x04;
    else
        return false;

    if (from->mapType_ == TreasureMapType_Regular)
        to->DiscoveryStateAndMapTypeAndUnknown |= 0x08;
    else if (from->mapType_ == TreasureMapType_Legacy)
        to->DiscoveryStateAndMapTypeAndUnknown |= 0x10;
    else
        return false;

    VectorizedInvertedMemcpy(from->discoveredBy_, to->DiscoveredBy, 10);
    VectorizedInvertedMemcpy(from->clearedBy_, to->ClearedBy, 10);
    to->Location = from->mapLocation_;

    if (from->discoveredTreasures_[0])
        to->TreasureDiscoveryFlags |= 0x01;
    if (from->discoveredTreasures_[1])
        to->TreasureDiscoveryFlags |= 0x02;
    if (from->discoveredTreasures_[2])
        to->TreasureDiscoveryFlags |= 0x04;

    if (from->mapType_ == TreasureMapType_Regular)
    {
        to->QualityOrLegacyBossID = from->regular_.quality_;
        to->LegacyBossLevel = 0;
        to->SeedOrMinTurns = from->regular_.seed_;
    }
    else if (from->mapType_ == TreasureMapType_Legacy)
    {
        to->QualityOrLegacyBossID = from->legacy_.bossID_;
        to->LegacyBossLevel = from->legacy_.level_;
        to->SeedOrMinTurns = from->legacy_.minTurns_;
    }

    return true;
}

// USA: func_020a3ec0
// JPN: func_020a5bfc
unsigned int RandATRangeModular(unsigned int minimum, unsigned int maximum)
{
    return minimum + ((unsigned int)rand() % (maximum - minimum + 1));
}