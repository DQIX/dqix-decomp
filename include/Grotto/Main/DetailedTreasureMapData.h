#pragma once

#include <globaldefs.h>
#include "TreasureMapMetadata.h"

#ifdef jpn
void RemoveFurigana(const char* src, char* dst);
#endif

bool IsMonsterIDLegacyBoss(unsigned short id);

// The output format here is pretty weird, it's a 4 byte struct of the form
// u8 mapType (1 = regular, 2 = legacy)
// u8 legacyBossID (0 if not a legacy map)
// u16 isLegacy (0 = regular, 1 = legacy but it's 16 bit...?)
bool GetTreasureMapTypeFromItemID(unsigned short itemID, unsigned char* out);

struct LegacyBossStats
{
    int maxHP;
    int maxMP;
    unsigned short agility;
    unsigned short attack;
    unsigned short defense;
    char pad_e[2];
    int alternateVersion; // 1, 2 or 3
    int rewardExp;
    // might be signed. only the last 16 bytes are used
    unsigned int rewardGold;
    unsigned char dropListIndex;
    bool newDropListAtNextLevel;
    unsigned char numLevelUpMoves;
    struct LevelUpMove
    {
        unsigned short moveID;
        unsigned char level;
        bool announceLearn;
    } levelUpMoves[10];
};

class DetailedTreasureMapData
{
public:
    class RegularMapData
    {
    public:
        unsigned short seed_;
        unsigned char quality_;
        // Related to unknown_66. It's always 0 but the code allows a random chance
        // to be 1-12 based on the seed (but in practice the chance is 0% for each).
        // I don't know what it does if it's nonzero.
        char unknown_4f_;
        unsigned char environ_; // caves, ruins, ice, water, fire as 1,2,3,4,5 resp.
        unsigned char floorCount_;
        unsigned char startingMonsterRank_;
        unsigned char bossID_; // 1 to 12
        unsigned short bossMonsterID_; // the number in square brackets in yabd's bestiary
        // looks like it gets populated with a random valid chest rank
        // for each monster rank, but never gets used
        char maybeUnusedChestRanks_[12];
        unsigned char prefix_;
        unsigned char suffix_;
        unsigned char localeRank_;
        unsigned char level_;
        char unknown_66_; // was always 1 in the grottos I checked

#if defined(usa)
        char nameNoLevel_[64];
        char levelString_[8];
        char topScreenName_[64];
        // Does not include the <PAD_WAIT> command (that's added externally)
        char popupName_[64];
#elif defined(jpn)
        char nameNoLevel_[32];
        char prefixString_[32];
        char suffixString_[32];
        char localeString_[32];
        char levelString_[8];
        char topScreenName_[64];
        char popupName_[128];
#endif

    public:
        void Populate(unsigned short seed, unsigned char quality);

    private:
        void GenerateUnknownData();
        void GenerateEnviron();
        void GenerateFloorCount();
        void GenerateMonsterRank();
        void GenerateBoss();
        void GenerateUnusedChestRanks();
        void GeneratePrefix();
        void GenerateSuffix();
        void GenerateLocaleRank();

        void GenerateNameBuffers();
        void GeneratePopupName();
    };

    class LegacyBossMapData
    {
    public:
        unsigned char bossID_;
        unsigned short bossMonsterID_; 
        // holds the ids of the 1A, 2A, 3A versions in yabd bestiary
        unsigned short alternateVersionIDs_[3];
        unsigned char level_;
        unsigned short minTurns_;
        // could be 24 bytes with padding
        // in JPN version, this is stored with furigana decorations
        char bossName_[26];
        LegacyBossStats stats_;

#if defined(usa)
        // All stored in the 'markup' encoding, e.g. using <1> for apostrophe
        char mapNameNoLevel_[64]; // e.g. "Baramos<1>s Map"
        char mapNameNoLevel_v2_[32]; // same as above, not sure what the difference is
        char seeminglyEmptyBuffer_[32];
        char mapLevelString_[8]; // e.g. "Lv. 99"
        char topScreenName_[64]; // "Baramos<1>s Map Lv. 99"
        char popupName_[64]; // e.g. "Baramos Lv. 99" (what pops up on entering the grotto)
#elif defined(jpn)
        // interpretation of this might be dodgy as I don't speak japanese
        char mapNameNoLevel_[32]; // e.g. "Baramos no chizu"
        char bossNameGenitive_[32]; // e.g. "Baramos no"
        char fixedStringChizu_[32]; // always holds "chizu" (means map?)
        char mapLevelString_[8]; // e.g. "Lv 99"
        char topScreenName_[64]; // e.g. "Baramos no chizu Lv 99"
        char popupName_[128]; // e.g. "Baramos Lv 99 no chizu" (pops up on entering the grotto)
#endif

    public:
        unsigned short MaybeGetCurrentAlternateID() const;
        void Populate(unsigned char bossID, unsigned char level, unsigned short minTurns);

        void WriteMapLevelString();
        bool CanUseLevelUpMove(unsigned short moveID);
        // if filter is 0 or 1, filters based on announceLearn
        unsigned short GetLearnedMove(unsigned char atLevel, int filter);
    };

    unsigned char discoveryState_;
    unsigned char mapType_;
    // Stored in the DQ9 string encoding. 12 bytes get zeroed out but
    // only 10 bytes get copied in
    char discoveredBy_[12];
    char clearedBy_[12];
    char pad_1a[2];
    // coordinates of the model (you spawn at offset (0, 0, +8192) on exiting)
    int entranceZoneID_;
    int entranceX_;
    int entranceY_;
    int entranceZ_;
    unsigned char mapLocation_; // 1 to 150
    char mapImageName_[16]; // e.g. "mapt_005"
    bool discoveredTreasures_[3];
    unsigned short treasureItemIDs_[3];
    unsigned char treasureDropRates_[3];
    char pad_49[3];

    union
    {
        RegularMapData regular_;
        LegacyBossMapData legacy_;
    };

public:
    void Clear();
    void BlankFunction() const; // possibly returns this, but return value never used

    // Called on the overall struct, but only does anything for a legacy
    // boss map (and maybe only gets called then?)
    bool UpdateFollowingCompletion(bool levelledUp, unsigned short numTurns);

    unsigned int GetLevel() const;

    void LoadLegacyBossStats(bool compute, const unsigned char* providedArchive);
    void LoadTreasures();
};