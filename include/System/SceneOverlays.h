#pragma once

// There are 6 slots for the 35 overlays, each gets loaded to a particular
//   slot 0  battle, event, topmenu, shisetsu, menucallback, equipmenu, renkin,
//   slot 1  debug, jourecode, charamake2, pitfall, menusys, prof, skillup
//   slot 2  subjugation, charaview, movieview, gamemain, mapjump, readerror, title, charamake
//   slot 3  sub_debug, sub_menu, sub_battle, sub_round, sub_command,
//           sub_staffroll, sub_hoge, sub_makescn, sub_libmb
//   slot 4  wifi, sound
//   slot 5  bgload1, bgload2

// data_01ffd364 is an array of 8? banks, each bank containing 6 slots
// data_01ffd348 holds the current bank index - as far as I can tell it's always 0?
// Maybe other functions can change it but they got optimized out at link time

struct SceneEntry
{
    int group;
    const char* name;
};

#define SCENE_COUNT 35

// USA: func_020a18f4
bool IsSceneLoaded(unsigned int scene);
