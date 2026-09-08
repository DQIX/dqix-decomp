#pragma once

struct PrimaryCombatStats {
    unsigned short currHP;
    unsigned short currMP;
    unsigned short maxHP;
    unsigned short maxMP;
    unsigned short attack;
    unsigned short defense;
    unsigned short agility;
    unsigned short unk;
    unsigned int charm : 10;
    unsigned int magicalMight : 10;
    unsigned int magicalMending : 10;
};

struct BaseCombatStats {
    char unk[0x2C];
    struct PrimaryCombatStats primaryStats;
};

struct ModifiableCombatStats {
    struct PrimaryCombatStats primaryStats; // 0xE
    char unk1[0x44];
    signed int attackBuff : 3;
    signed int defenseBuff : 3;
    signed int agilityBuff : 3;
    signed int charmBuff : 3;
    signed int magicalMightBuff : 3;
    signed int magicalMendingBuff : 3;
};

struct CombatantStruct {
    /* 0x000 */ unsigned short flags;
    /* 0x002 */ char unk[0x14 - 0x2];
    // Status bits. The accessors at 0x02088840..0x02088a70 set and clear seven
    // of them, each paired with an entry in the two byte arrays below.
    /* 0x014 */ unsigned int statusFlags;
    // Bit 0x40 is read as a second precondition by the state accessors.
    /* 0x018 */ unsigned int stateFlags;
    /* 0x01c */ char unknown_0x1c[0x5f - 0x1c];
    // Applying a status writes a small constant here and zeroes the matching
    // counter; clearing it zeroes both. INFERRED as a duration in turns from
    // the values (4, 5 and 6) - not established.
    /* 0x05f */ unsigned char statusDurations[11];
    /* 0x06a */ char unknown_0x6a[0x82 - 0x6a];
    /* 0x082 */ unsigned char statusCounters[11];
    /* 0x08d */ char unknown_0x8d[0x134 - 0x8d];
    /* 0x134 */ struct BaseCombatStats* baseStats; // TODO: holds more general info than just stats
    /* 0x138 */ struct ModifiableCombatStats* currentStats; // includes things like buffs being applied
};

// The offsets below were read off the accessors at 0x02011518..0x0201165c,
// which reach 0x63e0 - so this struct is at least 25 KB, far past the part
// modelled above. Everything between the named fields is carried as opaque
// bytes. Note the padding is expressed relative to combatantList, so if that
// array's size is ever corrected the padding must be adjusted to keep the
// absolute offsets in the comments true.
struct BattleStruct {
    /* 0x0000 */ int unk0;
    /* 0x0004 */ int unk4;
    /* 0x0008 */ struct CombatantStruct* combatantList[0xe9]; // TODO: validate this size as well as this struct as a whole
    /* 0x03ac */ char unknown_0x3ac[0x468 - 0x3ac];
    // A sub-object whose address is handed out by GetWorldData - ZoneFeatures
    // and Zone3D both call it the world data, and pass it to the map record
    // lookup. Its extent is not established; it is bounded here only by the
    // next known field.
    /* 0x0468 */ char worldData[0x571d - 0x468];
    // Four entries and a count, read through a bounds check that returns -1.
    // INFERRED as a party roster from the shape - four slots is the party
    // size - but not established.
    /* 0x571d */ signed char rosterIds[4];
    /* 0x5721 */ unsigned char rosterCount;
    /* 0x5722 */ char unknown_0x5722[0x5ccc - 0x5722];
    // Read sign-extended from bit 0, so a signed one-bit field: 0 or -1.
    /* 0x5ccc */ signed int flag_0x5ccc : 1;
                 unsigned int unknown_0x5ccc_rest : 31;
    /* 0x5cd0 */ char unknown_0x5cd0[0x63d6 - 0x5cd0];
    /* 0x63d6 */ unsigned char value_0x63d6;
    /* 0x63d7 */ char unknown_0x63d7;
    /* 0x63d8 */ unsigned short value_0x63d8;
    /* 0x63da */ unsigned short value_0x63da;
    /* 0x63dc */ char unknown_0x63dc[0x63e0 - 0x63dc];
    // GetTreasureMapLanguageData / SetTreasureMapLanguageDataPtr reach this
    // through a raw offset, which is 0x6180 on jpn - so the offsets in this
    // comment column hold for usa only.
    /* 0x63e0 */ unsigned char* pTreasureMapLanguageData;
};
struct BattleStruct* GetBattleStruct();
struct CombatantStruct* GetCombatantFromList(struct BattleStruct* battleStruct, int id);
