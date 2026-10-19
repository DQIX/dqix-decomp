#pragma once

#include <std_library_functions.h>

#define REG_KEYINPUT (*(volatile unsigned short*)0x04000130)

// this seems to hold bits 0-3 of EXTKEYIN from the arm7 left-shifted
// 10 places (it's usually 0x2c00 but loses the 0x400 bit when you
// press X and the 0x800 bit when you press Y). Also bit 15 is set
// when the lid is down (from testing)
#define ARM7_KEYINDATA (*(volatile unsigned short*)0x027fffa8)

#define ARM7_TOUCHDATAA (*(volatile unsigned short*)0x027fffaa)
#define ARM7_TOUCHDATAB (*(volatile unsigned short*)0x027fffac)

inline CBool IsLidDown()
{
    return (ARM7_KEYINDATA & 0x8000) >> 15;
}