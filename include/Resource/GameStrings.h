#pragma once

// all of this is usa/eur only
#if !defined(jpn)

#include "StringList.h"

enum eGameString
{
    GameString_Nine = 0,
    GameString_WarriorShortName = 1,
    GameString_PriestShortName = 2,
    GameString_MageShortName = 3,
    GameString_MartialArtistShortName = 4,
    GameString_ThiefShortName = 5,
    GameString_MinstrelShortName = 6,
    GameString_GladiatorShortName = 7,
    GameString_ArmamentalistShortName = 8,
    GameString_PaladinShortName = 9,
    GameString_SageShortName = 10,
    GameString_LuminaryShortName = 11,
    GameString_RangerShortName = 12,
    GameString_GuardianShortName = 13,
    GameString_Dead = 14,
    GameString_Poisoned = 15,
    GameString_Cursed = 16,
    GameString_Items = 17,
    GameString_Equip = 18,
    GameString_Attributes = 19,
    GameString_SpellsAndAbilities = 20,
    GameString_BattleRecords = 21,
    GameString_Misc = 22,
    GameString_Fight = 23,
    GameString_Examine = 24,
    GameString_Flee = 25,
    GameString_WholeParty = 26,
    GameString_Yes = 27,
    GameString_No = 28,
    GameString_Accept = 29,
    GameString_Decline = 30,
    GameString_CommunicationErrorMessage = 31,
    GameString_CharacterWaitingToJoinBattle = 32,
    GameString_CharacterJoinsBattle = 33,
    GameString_CharacterBeginsBattle = 34,
    GameString_CombatAlreadyConcluded = 35,
    GameString_CharacterIsConnecting = 36,
    GameString_CharacterTooDeadToTalk = 37,
    GameString_Whistle = 38,
    GameString_ButNothingHappens = 39,
    GameString_CharacterCantOpenChest = 40,
    GameString_ChestLocked = 41,
    GameString_ChestAmbush = 42,
    GameString_UnlockChest = 43,
    GameString_ChestEmpty = 45,
    GameString_Cannibox = 46,
    GameString_Mimic = 47,
    GameString_PandorasBox = 48,
    GameString_CastSafePassage = 49,
    GameString_VanishWearsOff = 50,
    GameString_HolyWaterWearsOff = 51,
    GameString_WaitingForHeroToFinishWatching = 52,
    GameString_BangHead = 57,
    GameString_Disembark = 58,
    GameString_SwitchToInsideBoat = 59,
    GameString_CantControlThisShip = 60,
    GameString_CantControlThisStarflight = 61,
    GameString_EvacuateImmediately = 62,
    GameString_SpotGrotto = 63,
    GameString_AcquireTreasureMap = 64,
    GameString_TooManyTreasureMaps = 65,
    GameString_VisitWhichRoom = 66,
    GameString_TakeLiftToRoyalSuites = 67,
    GameString_ErinnGreeting = 68,
    GameString_TreasureMap = 69,
    GameString_NoBooksOfInterest = 83,
    GameString_AcquireItem = 84,
    GameString_Boss = 85,
    GameString_Alchemy = 86,
    GameString_WaitingForHeroToFinishTalking = 200,
    GameString_HP = 201,
    GameString_MP = 202,
    GameString_None = 1000,
    GameString_Error1001 = 1001,
    GameString_CapitalizedSpeakerName = 1002,
    GameString_Val1 = 1003,
    GameString_PageNumberTemplate = 1004,
    GameString_GoldAmount = 1009,
    GameString_Error1010 = 1010,
    GameString_Lv = 1011,
    GameString_AcquireGold = 1012
};

// doesn't really do anything, you need to allocate memory with
// ReserveGameStringMemory() and then call LoadGameStrings()
void InitializeGameStrings();
void DestroyGameStrings();

void ReserveGameStringMemory(void* data, unsigned int length);
bool LoadGameStrings();

const char* GetGameString(eGameString id);
const char* GetArticleTemplate(int id);

#endif