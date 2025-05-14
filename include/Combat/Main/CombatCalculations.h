#pragma once

int RoundUp(float value);
float CalculatePhysicalDamage(int attack, int defense, int* variance);
float CalculateAttackBuffMultiplier(signed char buffLevel);
float CalculateDefenseBuffMultiplier(signed char buffLevel);
float CalculateAgilityBuffMultiplier(signed char buffLevel);
float CalculateCharmBuffMultiplier(signed char buffLevel);
float CalculateMagicalMightBuffMultiplier(signed char buffLevel);
float CalculateMagicalMendingBuffMultiplier(signed char buffLevel);