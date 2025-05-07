#pragma once

int RoundUp(float value);
float CalculatePhysicalDamage(int attack, int defense, int* variance);
float CalculateAttackBuffMultiplier(int buffLevel);
float CalculateDefenseBuffMultiplier(int buffLevel)