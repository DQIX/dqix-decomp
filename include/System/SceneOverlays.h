#pragma once

// The game's scenes, and the overlays behind them.
//
// data_020e8f20 is a table of 35 {group, name} pairs running 0x020e8f20 ..
// 0x020e9038. The names are the game's own:
//
//   group 0  battle event topmenu shisetsu menucallback equipmenu
//   group 1  renkin debug jourecode charamake2 pitfall menusys prof skillup
//   group 2  subjugation charaview movieview gamemain mapjump readerror title
//   group 3  charamake sub_debug sub_menu sub_battle sub_round sub_command
//            sub_staffroll sub_hoge sub_makescn sub_libmb
//   group 4  wifi sound
//   group 5  bgload1 bgload2
//
// A group is an overlay slot: at most one scene from each group is resident at
// a time, so 'title' and 'gamemain' (both group 2) cannot both be loaded.
//
// data_01ffd364 holds those slots - six signed bytes per bank, with the current
// bank index in data_01ffd348. A slot holds the scene index resident in it, so
// a scene is loaded exactly when its group's slot names it.

struct SceneEntry
{
    /* 0x00 */ int group;
    /* 0x04 */ const char* name;
};

#define SCENE_COUNT 35

extern "C"
{

// USA: func_020a18f4
bool IsSceneLoaded(unsigned int scene);

}
