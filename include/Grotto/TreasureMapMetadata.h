#pragma once

unsigned short GenerateMapLocation(unsigned int quality);
unsigned short GenerateNewMapQuality();

// This also seems to be the data that is stored to the save file (at location
// 0x5E8E as an array of these). 
class TreasureMapMetadata
{
    // Given the abundance of getter/setter functions, these are probably
    // private.
public:
    // Lowest 3 bits store the discovery state:
    // 0x01 = undiscovered, 0x02 = discovered, 0x04 = cleared.
    // Next 2 bits store the type (regular or legacy).
    // 0x08 = regular, 0x10 = legacy.
    // The 0x20 bit *might be* whether the grotto is currently selected
    // or not. But I'm not sure so I'm calling it unknown.
    unsigned char DiscoveryStateAndMapTypeAndUnknown;

    // Stored in the DQIX encoding
    char DiscoveredBy[10];
    // Stored in the DQIX encoding
    char ClearedBy[10];
    unsigned char Location;
    // 0x01: 100% drop discovered,
    // 0x02: common drop discovered,
    // 0x04: rare drop discovered
    unsigned char TreasureDiscoveryFlags;
    unsigned char QualityOrLegacyBossID;
    // might be char, might be signed. Seems the byte following this is always zero though
    unsigned char LegacyBossLevel; 
    unsigned short SeedOrMinTurns;

    // Might be a constructor. Generates an uncleared map
    void InitialiseAsNonLegacyMap(unsigned int quality, int seed);

    // Might be a constructor. Generates an uncleared map
    void InitialiseAsLegacyBossMap(unsigned int bossID, unsigned int level);

    // 1 = undiscovered, 2 = discovered, 3 = cleared
    void SetDiscoveryState(int state);
    int GetDiscoveryState() const;

    // Pass 1 for non-legacy, 2 for legacy
    void SetMapType(int type);
    // Returns 1 for non-legacy, 2 for legacy and 0 otherwise
    int GetMapType() const;

    void SetInitialByteUnknownBit();
    void ClearInitialByteUnknownBit();
    bool GetInitialByteUnknownBit() const;
    
    // The next function after this (func_020a6084) doesn't seem
    // related to this, so I think that's the last one. (It takes no parameters,
    // so if anything it would have to be a static function. It returns a fixed
    // memory address, could be some global instance of an array of these?
    // I looked at memory in-game and it didn't seem to suggest anything though)
};