#include "Grotto/TreasureMapMetadata.h"
#include "Grotto/RandATRangeModular.h"
#include "System/Memory.h"
#include "std_library_functions.h"
#include "Combat/Main/BattleList.h"
#include "Grotto/GrottoStruct.h"
#include <globaldefs.h>

#ifdef jpn
#define func_020100a8 func_0200ff04
#define func_0200ff1c func_0200fd78
#define func_02012fe4 func_02012dac
#endif

extern "C"
{
// Seems to return a u32 whose address is just past the end of the BattleStruct.
// Maybe BattleStruct is just the beginning of some larger struct?
unsigned int func_020100a8(BattleStruct*);

// Appears to index into the CombatantList and return the pointer after checking flags.
// For now we just return a char*, but should probably be a CombatantStruct*.
char* func_0200ff1c(BattleStruct*, unsigned int);

// This just returns some data. It's also being spam-called in TileFeatures.cpp,
// discarding the return value.
int func_02012fe4(void);
}

// USA: func_020a5cb8
// JPN: func_020a7a5c
unsigned short GenerateNewMapQuality()
{
    BattleStruct* battle = GetBattleStruct();
    char* maybeMainCharDataPtr = func_0200ff1c(battle, func_020100a8(battle));
    // Another pointless function call
    func_02012fe4();
    GrottoStruct* maybeGrottoDataPtr = GetGrottoStruct(battle);

#ifdef jpn
    #define MAIN_CHAR_DATA_PTR_OFFSET 0x144
#else
    #define MAIN_CHAR_DATA_PTR_OFFSET 0x150
#endif

    unsigned short maxCharLevel = 0;
    unsigned short maxNumRevocs = 0;
    for (int i = 0; i < 13; i++)
    {
        // WARNING: This is a minefield!
        // Even the slightest adjustments to this, while functionally irrelevant,
        // cause the compiler to assign different registers. As it stands, the
        // counter i is assigned to lr (weird choice).
        //
        // Also, decomp.me had this fully functional without declaring j as a
        // separate variable (just using a manual cast in the definitions of
        // level and revocCount) but on my machine my compiler didn't like that
        // and proceeded to put i into a different register.
        //
        // That said, if we define structs along the lines of
        // struct Inner { char unknown[0x16c]; unsigned short levels[13]; unsigned char revocs[13]; };
        // struct Outer { char unknown[0x150]; Inner* inner; };
        // and make maybeMainCharDataPtr of type Outer*, then we can use
        // inner->levels[j] and inner->revocs[j] and it still works. 
        unsigned char j = i;
        // beware pointer trickery, this is really offset 0x16c + 2*j
        unsigned short level = *(*(unsigned short**)(maybeMainCharDataPtr + MAIN_CHAR_DATA_PTR_OFFSET) + 0xb6 + j);  
        unsigned short revocCount = *(*(unsigned char**)(maybeMainCharDataPtr + MAIN_CHAR_DATA_PTR_OFFSET) + 0x186 + j);
        if (level > maxCharLevel)
            maxCharLevel = level;
        if (revocCount > maxNumRevocs)
            maxNumRevocs = revocCount;
    }

    if (maxCharLevel > 99)
        maxCharLevel = 99;

    if (maxNumRevocs > 10)
        maxNumRevocs = 10;

    unsigned short quality;
    // Probably checking if regular map or legacy boss map?
    if (maybeGrottoDataPtr->unknown[9] == 2)
    {
        quality = maxCharLevel + maxNumRevocs * 5 + maybeGrottoDataPtr->unknown[7];
    }
    else
    {
        quality = (unsigned short)(1.5f * (float)maxCharLevel + 5.0f * (float)maxNumRevocs);
    }
    maybeGrottoDataPtr->unknown[9] = 0;
    float tenth = 0.1f * (float)quality;
    int quotient = 2 * (int)tenth + 1;
    quality += (int)((float)(rand() % quotient) - tenth);
    if (quality < 2)
        quality = 2;
    if (quality > 248)
        quality = 248;
    return quality;
}

// USA: func_020a5e10
// JPN: func_020a7bb4
unsigned short GenerateMapLocation(unsigned int quality)
{
    if (quality >= 81 && quality <= 248)
        return RandATRangeModular(1, 150);
    if (quality >= 51 && quality <= 80)
        return RandATRangeModular(1, 131);
    return RandATRangeModular(1, 47);
}

// USA: func_020a5e7c
// JPN: func_020a7c20
ARM void TreasureMapMetadata::InitialiseAsNonLegacyMap(unsigned int quality, int seed)
{
    VectorizedMemset(this, 0, 28);
    SetDiscoveryState(DiscoveryState_Undiscovered);
    SetMapType(TreasureMapType_Regular);
    if (quality >= 2 && quality <= 248)
    {
        QualityOrLegacyBossID = (unsigned char)quality;
    }
    else
    {
        QualityOrLegacyBossID = (unsigned char)GenerateNewMapQuality();
    }

    if (seed == 0)
    {
        SeedOrMinTurns = rand() % 0xffff;
    }
    else
    {
        SeedOrMinTurns = seed;
    }

    Location = (unsigned char)GenerateMapLocation(QualityOrLegacyBossID);
}

// USA: func_020a5efc
// JPN: func_020a7ca0
ARM void TreasureMapMetadata::InitialiseAsLegacyBossMap(unsigned int bossID, unsigned int level)
{
    VectorizedMemset(this, 0, 28);
    SetDiscoveryState(DiscoveryState_Undiscovered);
    SetMapType(TreasureMapType_Legacy);
    if (bossID == 0 || bossID > 13)
    {
        QualityOrLegacyBossID = 1;
    }
    else
    {
        QualityOrLegacyBossID = (unsigned char)bossID;
    }

    if (level == 0)
    {
        LegacyBossLevel = 1;
    }
    else if (level > 99)
    {
        LegacyBossLevel = 99;
    }
    else
    {
        LegacyBossLevel = (unsigned char)level;
    }
    SeedOrMinTurns = 0;
    
    int finalQuality = GenerateNewMapQuality();
    Location = (unsigned char)GenerateMapLocation(finalQuality);
}

// USA: func_020a5f88
// JPN: func_020a7d2c
ARM void TreasureMapMetadata::SetDiscoveryState(eDiscoveryState state)
{
    DiscoveryStateAndMapTypeAndUnknown &= 0xF8;
    if (state == 1)
    {
        DiscoveryStateAndMapTypeAndUnknown |= 0x01;
    }
    else if (state == 2)
    {
        DiscoveryStateAndMapTypeAndUnknown |= 0x02;
    }
    else if (state == 3)
    {
        DiscoveryStateAndMapTypeAndUnknown |= 0x04;
    }
}

// USA: func_020a5fd0
// JPN: func_020a7d74
ARM eDiscoveryState TreasureMapMetadata::GetDiscoveryState() const
{
    if (DiscoveryStateAndMapTypeAndUnknown & 0x01)
        return DiscoveryState_Undiscovered;
    if (DiscoveryStateAndMapTypeAndUnknown & 0x02)
        return DiscoveryState_Discovered;
    if (DiscoveryStateAndMapTypeAndUnknown & 0x04)
        return DiscoveryState_Cleared;
    return DiscoveryState_Invalid;
}

// USA: func_020a5ffc
// JPN: func_020a7da0
ARM void TreasureMapMetadata::SetMapType(eTreasureMapType type)
{
    DiscoveryStateAndMapTypeAndUnknown &= 0xe7;
    if (type == TreasureMapType_Regular)
    {
        DiscoveryStateAndMapTypeAndUnknown |= 0x08;
    }
    else if (type == TreasureMapType_Legacy)
    {
        DiscoveryStateAndMapTypeAndUnknown |= 0x10;
    }
}

// USA: func_020a6030
// JPN: func_020a7dd4
ARM eTreasureMapType TreasureMapMetadata::GetMapType() const
{
    if (DiscoveryStateAndMapTypeAndUnknown & 0x08)
        return TreasureMapType_Regular;
    if (DiscoveryStateAndMapTypeAndUnknown & 0x10)
        return TreasureMapType_Legacy;
    return TreasureMapType_Invalid;
}

// USA: func_020a6050
// JPN: func_020a7df4
ARM void TreasureMapMetadata::SetInitialByteUnknownBit()
{
    DiscoveryStateAndMapTypeAndUnknown |= 0x20;
}

// USA: func_020a6060
// JPN: func_020a7e04
ARM void TreasureMapMetadata::ClearInitialByteUnknownBit()
{
    DiscoveryStateAndMapTypeAndUnknown &= 0xdf;
}

// USA: func_020a6070
// JPN: func_020a7e14
ARM bool TreasureMapMetadata::GetInitialByteUnknownBit() const
{
    return DiscoveryStateAndMapTypeAndUnknown & 0x20;
}