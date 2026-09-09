#include "World/Zone3D.h"
#include "Combat/Main/BattleList.h"
#include "Filesystem/BackgroundLoader.h"
#include "Filesystem/NarcHandle.h"
#include "Filesystem/FileAccessor.h"
#include "Filesystem/LowNitroHandle.h"
#include "Filesystem/FileIO.h"
#include "Resource/GameResources.h"
#include "Graphics/NSBXX/NSBXX.h"

#if defined(jpn)
#define func_0200fdcc func_0200fc28
#define func_0200fddc func_0200fc38
#define func_02011584 func_020112f4
#define func_0201e248 func_0201dfd4
#define func_02013750 func_02013518
#define func_02013490 func_02013258
#define func_0201b5b0 func_0201b328
#define func_02053c6c func_02054fe4
#define func_0207a5b8 func_0207b3f0
#define func_0207b9cc func_0207c804
#define func_0207df50 func_0207ecd0
#define func_0208a9b4 func_0208b2a8
#define func_02094d00 func_02096950
#define func_02099950 func_0209b684
#define func_020de848 func_020e01c4

#define data_020ef0f0 data_020ef02c
#endif

extern "C"
{
    void* func_02011584(BattleStruct*);
    void func_02013454(void*);
    void* func_0200fdcc(BattleStruct*);
    void* func_0200fddc(BattleStruct*);

    void* func_02053c6c(void*);
    void func_0205e104(const char*, SafeAllocator*, const void*, unsigned int);

    // Texture functions
    void* func_0207df50(void*);
    void func_0207df90(void*);
    void func_0207dfac(void*);

    void* func_0208a9b4();
    void func_02094d00(void*);
    Zone3D_StructPtr_8* func_02099950(void*, unsigned short id);

    void func_020c9be0(); // abort() or similar
    void func_020de848(void*);

    void func_02013490(void*);
    void func_02013750(Zone3D*, bool);
    void func_02014414(Zone3D*, const void*, unsigned);
    void func_02014a24(Zone3D*, void*);

    // checks if zone id corresponds to a main floor of a grotto
    bool func_0201b5b0(int id);
    // checks if zone id corresponds to boss floor of a grotto
    bool func_0201b5d8(int id);

    void func_0201e248(void*);
}

extern char data_020ef0f0[]; // "data/map/maplist9.bin"
extern char data_020ef106[]; // "%s/Z0%dM01.ambl"
extern char data_020ef116[]; // "data/map"
extern char data_020ef11f[]; // "%s/Z0%dM99.ambl"
extern char data_020ef12f[]; // "%s/%s.ambl"
extern char data_020ef13a[]; // "ARC"
extern char data_020ef13e[]; // ".nsbtx"
extern char data_020ef145[]; // ".bmbl"
extern char data_020ef14b[]; // ".dat"
extern char data_020ef150[]; // ".bpos"
extern char data_020ef156[]; // "%s/Z0%dM01.amdj"
extern char data_020ef166[]; // "%s/Z0%dM99.amdj"
extern char data_020ef176[]; // "%s/%sb.amdj"
extern char data_020ef182[]; // "%s/%sa.amdj"
extern char data_020ef18e[]; // "%s/%s.amdj"
extern char data_020ef199[]; // ".bmdj"
extern char data_020ef19f[]; // "Z0%dM0100"
extern char data_020ef1a9[]; // "Z0%dM0101"
extern char data_020ef1b3[]; // "Z0%dM0102"
extern char data_020ef1bd[]; // "Z0%dM0103"
extern char data_020ef1c7[]; // "%s/ats_%c.ambl"
extern char data_020ef1d6[]; // "%s.bats"
extern char data_020ef1de[]; // "ARC:/%s"
extern char data_020ef1e6[]; // "."
extern char data_020ef1e8[]; // "nsbmd"
extern char data_020ef1ee[]; // "col2"
extern char data_020ef1f3[]; // "open"
extern char data_020ef1f8[]; // "open2"
extern char data_020ef1fe[]; // "close"
extern char data_020ef204[]; // "close2"
extern char data_020ef20b[]; // "/data/ani/d_%c%03d.spr"
extern char data_020ef222[]; // "tsuboware"
extern char data_020ef22c[]; // "ARC:%s"

void Zone3D::SwitchZone(unsigned short newID)
{
    BattleStruct* battle = GetBattleStruct();
    BackgroundLoader* loader = BackgroundLoader::GetInstance();

    void* uVar3 = func_02011584(battle);
    (void)func_ov017_0218b5b0();
    void* iVar4 = func_0200fddc(battle);

    pAllocator_68_ = pAllocator_4c_;
    pAllocator_68_->Reset();

    func_0207df50(unknown_ptr_50_);
    func_02013750(this, true);

    previousZoneID_ = currentZoneID_;
    currentZoneID_ = newID;

    textureImageMemory_ = 0;
    texturePaletteMemory_ = 0;
    unknown_424_ = 1;
    firstBMDJStruct_41c_ = 0;
    firstModel_418_ = NULL;
    unknown_476_ = 0;
    numChests_ = 0;
    unknown_82c_ = 0;
    unknown_474_ = 0;
    unknown_82c_ = 0; // why zero it twice?
    unknown_42c_ = 0;

    mapListLoadHandle_ = -1;
    unknown_434_ = -1;
    mapAMBLLoadHandle_ = -1;
    mapAMDJLoadHandle_ = -1;
    atsAMBLLoadHandle_ = -1;

    unknown_478_ = 0;
    unknown_47c_ = 0;
    unknown_834_ = 0;
    unknown_2820_ = 0;

    func_0201e248(substruct_6c_);

    substruct_c_.buffer1[0] = 0;
    substruct_c_.buffer2[0] = 0;
    substruct_c_.buffer3[0] = 0;
    substruct_c_.unknown_2a_ = 0x7fff;
    substruct_c_.unknown_2c_ = 0;
    substruct_c_.unknown_30_ = 10;
    substruct_c_.unknown_34_ = 0;
    substruct_c_.unknown_38_ = 0;
    substruct_c_.unknown_3c_ = 0;

    atmosphericEffects_.Reset();
    lighting_.Reset();
    func_020de848(&unknown_struct_2754_[0]);

    pUnknownStruct_8_ = func_02099950(uVar3, newID);
    unknown_4_ = pUnknownStruct_8_->unknown_2_;
    if (pUnknownStruct_8_->unknown_c_low_ == 0)
    {
        void* iVar5 = func_0200fdcc(battle);
        if (iVar5 != NULL)
        {
            void* iVar6 = func_02053c6c(iVar5);
            if (iVar6 != NULL)
                *(unsigned short*)((int)iVar6 + 0x566) = pUnknownStruct_8_->unknown_0_;
        }
    }

    *(bool*)((int)func_0208a9b4() + 0x9c) = pUnknownStruct_8_->unknown_c_high_ != 0;
    func_02094d00(&unknown_struct_2724_[0]);

    grottoTileMapData_420_ = NULL;

    if (func_0201b5b0(previousZoneID_))
    {
        grotto_.floorMap_.Clear();
    }

    if (func_0201b5b0(newID))
    {
        isInMainGrottoFloor_23b8_ = true;
        currentGrottoFloor_23ba_ = newID % 20;
        copyOfCurrentGrottoFloor_23bb_ = currentGrottoFloor_23ba_;
        int width = grotto_.CalculateAndStoreFloorWidth(currentGrottoFloor_23ba_);
        int height = grotto_.CalculateAndStoreFloorHeight(currentGrottoFloor_23ba_);

        grottoTileMapData_420_ = pAllocator_68_->Allocate(0x48 * 256);
        for (int i = 0; i < 256; i++)
        {
            func_02013490((char*)grottoTileMapData_420_ + i * 0x48);
        }
        grotto_.ClearGenerator(false);
        grotto_.AllocateGenerator(pAllocator_68_, false);
        grotto_.CalculateFloorMap(currentGrottoFloor_23ba_, width, height, NULL);
    }
    else
    {
        if (currentGrottoFloor_23ba_ != -1)
        {
            copyOfCurrentGrottoFloor_23bb_ = currentGrottoFloor_23ba_;
            position_23c0_ = *(Vector3i*)((int)iVar4 + 0x44);
            unknown_23cc_ = *(short*)((int)iVar4 + 0xae);
        }
        isInMainGrottoFloor_23b8_ = false;
        currentGrottoFloor_23ba_ = -1;
    }

    mapListLoadHandle_ = loader->QueueLoadFile(data_020ef0f0, NULL);
}