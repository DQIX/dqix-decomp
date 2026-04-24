#pragma once

#include <globaldefs.h>

struct DetailedTreasureMapData
{
    unsigned char discoveryState;
    unsigned char mapType;
    // Stored in the DQ9 string encoding. 12 bytes get zeroed out but
    // only 10 bytes get copied in
    char discoveredBy[12];
    char clearedBy[12];
    char probablyPadding_1A[2];
    // not sure about this, seemed correct for a couple of grottos but the
    // z coordinate was off by 8192. I guess it could be the location of the
    // grotto entrance model? You do spawn a bit below it when you exit.
    int entranceZoneID;
    int entranceX;
    int entranceY;
    int entranceZ;
    unsigned char mapLocation;
    char mapImageName[16]; // e.g. "mapt_005"
    bool discoveredTreasures[3];
    short treasureItemIDs[3]; // might be unsigned?
    unsigned char treasureDropRates[3];
    char unknown_49[3]; // might just be padding

    union {
    struct RegularMapData
    {
        short seed; // might technically be unsigned
        unsigned char quality;
        char padding_4F[1];
        // the rest of these are probably unsigned too
        char environ; // caves, ruins, ice, water, fire as 1,2,3,4,5 resp.
        unsigned char floorCount;
        char startingMonsterRank;
        char bossID; // 1 to 12
        // these are probably chars, or maybe a short followed by 12 chars.
        // the first byte is typically quite large (0xD8 in usugura)
        // and the rest are around 1-10
        char unknown_54[14];
        char prefix;
        char suffix;
        char maybeLocale; // needs more checking
        unsigned char level;
        char unknown_66; // was always 1 in the grottos I checked

#ifndef jpn
        // This is still pretty speculative, but from a quick glance at
        // func_020a51b0 and looking at memory while in a grotto, I think
        // it's something like:
        char buffer1[64]; // holds grotto name without level
        char buffer2[8]; // level string, e.g. "Lv. 1"
        char buffer3[64]; // full grotto name
        char popupName[64]; // full grotto name but used in the popup msg
                            // when you enter / load a quicksave
#else
        // this is probably not what's actually going on, but it makes
        // this object the right size for memset calls. Probably either the
        // buffers are resized or there are some extra ones.
        char unknownJpnBuffers[0xC8];
        char popupName[64];
#endif
    } regular;

    struct LegacyBossMapData
    {
        char bossID;
        short unknown_4E[4];
        unsigned char level;
        unsigned short minTurns;
        char bossName[26]; // unsure of length, could be zeros at end for another reason
        // amazingly these are actually used quantities, if you modify them
        // on entering the grotto with a cheat, it changes their stats
        // when you fight them 
        int bossHP;
        int bossMP;
        unsigned short bossAgility;
        unsigned short bossAttack;
        unsigned short bossDefense;
        char padding_82[2];
        int unknown_84;
        int rewardExp;
        unsigned short rewardGold;
        char unknown_8E[0x2E];

#ifndef jpn
        // All stored in the 'markup' encoding, e.g. using <1> for apostrophe.
        // Speculative based on looking at memory in a legacy grotto and
        // a glance at func_020a425c
        char mapNameNoLevel[64]; // e.g. "Estark<1>s Map"
        char mapNameNoLevel_v2[64]; // same as above, not sure what the difference is
        char mapLevelString[8]; // e.g. "Lv. 99"
        char mapName[64]; // "Estark<1>s Map Lv. 99"
        char popupName[64]; // e.g. "Estark Lv. 99" (what pops up on entering the grotto)
#else
        // really no idea, I just looked at func_020a6050 (JPN) to guess
        // the sizes and positions
        char jpnBuffer1[32];
        char jpnBuffer2[32];
        char jpnBuffer3[32];
        char jpnBuffer4[8];
        char jpnBuffer5[64];
        char popupName[64];
        char jpnBuffer6[64];
#endif
    } legacy;
    };

};