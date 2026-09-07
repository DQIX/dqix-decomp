#include "World/Zone3D.h"
#include "Combat/Main/BattleList.h"
#include "Filesystem/BackgroundLoader.h"
#include "Filesystem/NarcHandle.h"
#include "Filesystem/FileAccessor.h"
#include "Filesystem/LowNitroHandle.h"
#include "Filesystem/FileIO.h"
#include "Grotto/Overlay_17/Struct44C8.h"
#include "Graphics/NSBXX/NSBXX.h"

// we still need to include this file because of Vector3i::operator= being
// implicitly defined here, so use this to include all the other functions 
// defined after it (a very incomplete list atm)
// #define ZONE3D_EXPERIMENTAL

#if defined(jpn)
#define func_0200fdcc func_0200fc28
#define func_0200fddc func_0200fc38
#define func_02011584 func_020112f4
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

    bFeatures_.Reset();

    mapListInfo_.maybeModelName[0] = 0;
    mapListInfo_.buffer2[0] = 0;
    mapListInfo_.buffer3[0] = 0;
    mapListInfo_.unknown_2a = 0x7fff;
    mapListInfo_.unknown_2c = 0;
    mapListInfo_.unknown_30 = 10;
    mapListInfo_.worldRotation = 0;
    mapListInfo_.unknown_38 = 0;
    mapListInfo_.unknown_3c = 0;

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
        grotto_.floorMap.Clear();
    }

    if (func_0201b5b0(newID))
    {
        isInMainGrottoFloor_23b8_ = true;
        currentGrottoFloor_23ba_ = newID % 20;
        copyOfCurrentGrottoFloor_23bb_ = currentGrottoFloor_23ba_;
        int width = grotto_.CalculateAndStoreFloorWidth(currentGrottoFloor_23ba_);
        int height = grotto_.CalculateAndStoreFloorHeight(currentGrottoFloor_23ba_);

        grottoTileMapData_420_ = (GrottoTileData*)pAllocator_68_->Allocate(sizeof(GrottoTileData) * 256);
        for (int i = 0; i < 256; i++)
        {
            func_02013490(&grottoTileMapData_420_[i]);
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

// implicitly defined Vector3i::operator=(const Vector3i&)

#ifdef ZONE3D_EXPERIMENTAL

bool Zone3D::ProcessMaplist9()
{
    if (mapListLoadHandle_ < 0)
        return true;
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    if (loader->GetTaskStatus(mapListLoadHandle_) == 0)
        return false;
    
    void* script;
    unsigned int length;
    loader->GetLoadedFileByID(mapListLoadHandle_, &script, &length);
    LoadZoneInfoFromMapListScript(currentZoneID_, &mapListInfo_, script, length);
    loader->RemoveTask(mapListLoadHandle_);
    mapListLoadHandle_ = -1;
    LoadMapAMBL();
    return true;
}

void Zone3D::LoadMapAMBL()
{
    BackgroundLoader* loader = BackgroundLoader::GetInstance();

    char filenameBuffer[20];

    if (func_0201b5b0(currentZoneID_))
    {
        int environ = grotto_.GetActiveGrottoEnviron();
        if (environ == 0)
            environ = 1;
        if (environ > 5)
            environ = 5;
        sprintf(filenameBuffer, data_020ef106, data_020ef116, environ);
    }
    else if (func_0201b5d8(currentZoneID_))
    {
        int environ = grotto_.GetActiveGrottoEnviron();
        sprintf(filenameBuffer, data_020ef11f, data_020ef116, environ);
    }
    else
    {
        sprintf(filenameBuffer, data_020ef12f, data_020ef116, pUnknownStruct_8_->mapShortName_);
    }
    mapAMBLLoadHandle_ = loader->QueueLoadFile(filenameBuffer, NULL);
}

bool Zone3D::UnpackMapAMBL()
{
    if (mapAMBLLoadHandle_ < 0)
        return true;

    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    if (loader->GetTaskStatus(mapAMBLLoadHandle_) == 0)
        return false;

    // If we get here, the loading finished but was not successful
    if (loader->GetDetailedTaskStatus(mapAMBLLoadHandle_) != BackgroundLoader::TaskStatus_Complete)
    {
        loader->RemoveTask(mapAMBLLoadHandle_);
        mapAMBLLoadHandle_ = -1;
        return true;
    }
    
    void* amblData;
    unsigned int amblFilesize;
    
    loader->GetLoadedFileByID(mapAMBLLoadHandle_, &amblData, &amblFilesize);

    for (int pass = 0; pass < 2; pass++)
    {
        NarcHandle narc;
        if (narc.Initialize(data_020ef13a, (const unsigned char*)amblData))
        {
            NitroVM vm;
            unsigned int fileID = 0;
            NitroVM_Initialize(&vm);
            while (PrepareReadFileInNARCByID(&vm, &narc, fileID))
            {
                char innerFilePath[80];
                NitroVM_WriteOutFilePath(&vm, innerFilePath, 80);
                
                const char* extension = strrchr(innerFilePath, '.');
                if (extension == NULL)
                {
                    NitroVM_FinishRead(&vm);
                    fileID++;
                    continue;
                }
                unsigned int innerFilesize = vm.regbase_abc.c.u32 - vm.regbase_abc.b.u32;
                NitroVM_FinishRead(&vm);
                const void* innerFilePtr = narc.GetFileByIndex(fileID);
                if (pass == 0)
                {
                    // nsbtx file (we can have multiple of these)
                    if (strcmp(data_020ef13e, extension) == 0)
                        ProcessNSBTXFile(innerFilePtr, innerFilesize, innerFilePath);
                }
                else if (pass == 1)
                {
                    // bmbl file
                    if (strcmp(data_020ef145, extension) == 0)
                        ProcessBMBLFile(innerFilePtr, innerFilesize);
                    // dat file
                    else if (strcmp(data_020ef14b, extension) == 0)
                    {
                        SafeAllocator* alloc = pAllocator_68_;
                        unsigned int decompressedSize;
                        const void* decompressed = DecompressLZ77FileIntoScratchSpace(*alloc, innerFilePtr, decompressedSize);
                        // this call is responsible for setting the top-screen map
                        func_0205e104(mapListInfo_.maybeModelName, alloc, decompressed, decompressedSize);
                    }
                    // bpos file. From testing these seem to be a grotto thing
                    else if (strcmp(data_020ef150, extension) == 0)
                        ProcessBPOSFile(innerFilePtr, innerFilesize);
                }
                fileID++;
            }
            narc.Destroy();
        }
        if (pass == 0)
        {
            unsigned int allocSize = pAllocator_4c_->GetMaxPossibleAllocation();
            void* memory = pAllocator_4c_->Allocate(allocSize);
            if (memory == NULL)
                func_020c9be0();
            internalAllocator_.ResetAllocatorPointer();
            internalAllocator_.CreateTypeA(memory, allocSize);
            pAllocator_68_ = &internalAllocator_;
            internalAllocator_.Reset();
        }
    }
    loader->RemoveTask(mapAMBLLoadHandle_);
    mapAMBLLoadHandle_ = -1;
    QueueLoadATS_AMBL();
    return true;
}

bool Zone3D::ProcessBMBLFile(const void* filedata, unsigned int /*filesize*/)
{
    SafeAllocator* allocator = pAllocator_68_;
    unsigned int decompressedLength;
    void* decompressed = DecompressLZ77FileIntoScratchSpace(*allocator, filedata, decompressedLength);
    Zone3D_StructPtr_8* ptr8 = pUnknownStruct_8_;

    bFeatures_.Reset();
    // run another script with opcode table at 0x020ef388
    bFeatures_.LoadFromScript(allocator, decompressed, decompressedLength);
    pUnknownStruct_8_ = ptr8; // why?
    return true;
}

bool Zone3D::ProcessBPOSFile(const void* filedata, unsigned int /*filesize*/)
{
    SafeAllocator* allocator = pAllocator_68_;
    unsigned int decompressedLength;
    void* decompressed = DecompressLZ77FileIntoScratchSpace(*allocator, filedata, decompressedLength);
    Zone3D_StructPtr_8* ptr8 = pUnknownStruct_8_;

    bFeatures_.LoadFromScript(allocator, decompressed, decompressedLength);
    pUnknownStruct_8_ = ptr8; // why?
    return true;
}

bool Zone3D::ProcessBATSFile(const void* filedata, unsigned int /*filesize*/)
{
    SafeAllocator* allocator = pAllocator_68_;
    unsigned int decompressedLength;
    void* decompressed = DecompressLZ77FileIntoScratchSpace(*allocator, filedata, decompressedLength);
    lighting_.Initialize();
    lighting_.LoadFromScript(decompressed, decompressedLength, allocator);
    return true;
}

bool Zone3D::ProcessNSBTXFile(const void* filedata, unsigned int filesize, const char* filename)
{
    SafeAllocator* allocator = pAllocator_68_;
    void* graphicsPtr = unknown_ptr_50_;

    Model3DListNode* modelNode = (Model3DListNode*)allocator->Allocate(sizeof(Model3DListNode));
    if (modelNode != NULL)
    {
        modelNode->model_.Clear();
        modelNode->filename_ = NULL;
        modelNode->pNext_ = NULL;
        char* newFilenameBuffer = (char*)allocator->Allocate(strlen(filename) + 1);
        modelNode->filename_ = newFilenameBuffer;
        if (newFilenameBuffer != NULL)
        {
            strcpy(newFilenameBuffer, filename);
            modelNode->pNext_ = firstModel_418_;
            firstModel_418_ = modelNode;
            unsigned int decompressedLength;
            void* decompressed = DecompressLZ77FileIntoScratchSpace(*allocator, filedata, decompressedLength);
            if (decompressed != NULL)
            {
                func_0207df90(graphicsPtr);
                modelNode->model_.SetRawFile(decompressed, decompressedLength);
                modelNode->model_.ClearRawFileCache();
                modelNode->model_.ProcessRawFile(Model3D::TextureStagingMode_Immediate);
                func_0207dfac(graphicsPtr);
                NSBXXTex* texture = modelNode->model_.GetTEX0();
                if (texture != NULL)
                {
                    textureImageMemory_ += NSBXX_Tex_GetBlock1Length(texture);
                    texturePaletteMemory_ += NSBXX_Tex_GetBlock4Length(texture);
                }
                bool success = false;
                if (texture != 0)
                {
                    // bit weird, but I guess block 1 starts right after the metadata ends
                    unsigned int textureMetadataLength = texture->block1Offset_;
                    NSBXXTex* copyOfpVar5 = (NSBXXTex*)allocator->Allocate(textureMetadataLength);
                    if (copyOfpVar5 != NULL)
                    {
                        memcpy(copyOfpVar5, texture, textureMetadataLength);
                        modelNode->model_.SetTEX0(copyOfpVar5);
                        success = true;
                    }
                }
                if (!success)
                    modelNode->model_.Clear();
            }
        }
    }
    return true;
}

void Zone3D::LoadMapAMDJ()
{
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    char filenameBuffer[20];
    if (func_0201b5b0(currentZoneID_))
    {
        int environ = grotto_.GetActiveGrottoEnviron();
        if (environ == 0)
            environ = 1;
        if (environ > 5)
            environ = 5;
        sprintf(filenameBuffer, data_020ef156, data_020ef116, environ);
    }
    else if (func_0201b5d8(currentZoneID_))
    {
        int environ = grotto_.GetActiveGrottoEnviron();
        sprintf(filenameBuffer, data_020ef166, data_020ef116, environ);
    }
    else
    {
        if (currentZoneID_ == 10000 || currentZoneID_ == 10100)
        {
            if (unknown_42c_ == 0)
            {
                sprintf(filenameBuffer, data_020ef176, data_020ef116, pUnknownStruct_8_->mapShortName_);
                unknown_42c_++;
            }
            else if (unknown_42c_ == 1)
            {
                sprintf(filenameBuffer, data_020ef182, data_020ef116, pUnknownStruct_8_->mapShortName_);
                unknown_42c_++;
            }
        }
        else
        {
            sprintf(filenameBuffer, data_020ef18e, data_020ef116, pUnknownStruct_8_->mapShortName_);
        }
    }
    mapAMDJLoadHandle_ = loader->QueueLoadFile(filenameBuffer, NULL);
}

bool Zone3D::UnpackMapAMDJ()
{
    if (mapAMDJLoadHandle_ < 0)
        return true;

    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    if (loader->GetTaskStatus(mapAMDJLoadHandle_) == 0)
        return false;

    // If we get here, loading finished but was not successful
    if (loader->GetDetailedTaskStatus(mapAMDJLoadHandle_) != BackgroundLoader::TaskStatus_Complete)
    {
        loader->RemoveTask(mapAMDJLoadHandle_);
        mapAMDJLoadHandle_ = -1;
        return true;
    }

    void* amdjData;
    unsigned int amdjFilesize;
    loader->GetLoadedFileByID(mapAMDJLoadHandle_, &amdjData, &amdjFilesize);
    NarcHandle narc;
    if (narc.Initialize(data_020ef13a, (unsigned char*)amdjData))
    {
        NitroVM vm;
        unsigned int fileID = 0;
        NitroVM_Initialize(&vm);
        while (PrepareReadFileInNARCByID(&vm, &narc, fileID))
        {
            char innerFilePath[80];
            NitroVM_WriteOutFilePath(&vm, innerFilePath, 80);
            
            const char* extension = strrchr(innerFilePath, '.');
            if (extension == NULL)
            {
                NitroVM_FinishRead(&vm);
                fileID++;
                continue;
            }
            unsigned int innerFilesize = vm.regbase_abc.c.u32 - vm.regbase_abc.b.u32;
            NitroVM_FinishRead(&vm);
            const void* innerFilePtr = narc.GetFileByIndex(fileID);
            if (strcmp(data_020ef199, extension) == 0)
            {
                int numIterations = bFeatures_.arraySize64_;
                for (int i = 0; i < numIterations; i++)
                {
                    ZoneFeatures::Opcode64Entry* bstr = bFeatures_.GetOpcode64Entry(i);
                    if (strstr(innerFilePath, bstr->string_10))
                        ProcessBMDJFile(innerFilePtr, innerFilesize, bstr);
                }
            }
            fileID++;
        }
        for (Zone3D_BMDJStruct* item = firstBMDJStruct_41c_; item != NULL; item = item->pNext_)
        {
            func_02014a24(this, item);
            if (unknown_42c_ == 2) 
                break;
        }
        narc.Destroy();
    }
    loader->RemoveTask(mapAMDJLoadHandle_);
    mapAMDJLoadHandle_ = -1;
    if (unknown_42c_ == 1)
    {
        LoadMapAMDJ();
        return false;
    }

    if (mapListInfo_.buffer2[0] != '\0')
        atmosphericEffects_.LoadArchive(mapListInfo_.buffer2);
    return true;
}

bool Zone3D::ProcessBMDJFile(const void* filedata, unsigned int filesize, ZoneFeatures::Opcode64Entry* misc)
{
    SafeAllocator* allocator = pAllocator_68_;
    Zone3D_BMDJStruct* newStruct = (Zone3D_BMDJStruct*)allocator->Allocate(sizeof(Zone3D_BMDJStruct));
    if (newStruct == NULL)
        return false;

    func_02013454(newStruct);
    newStruct->unknown_0_ = misc->unk_0;
    newStruct->vec_48_ = misc->vector_4.vec;
    unsigned int decompressedLength;
    void* decompressed = DecompressLZ77FileIntoScratchSpace(*allocator, filedata, decompressedLength);
    if (decompressed == NULL)
        return false;

    newStruct->scriptData_.Load(allocator, decompressed, decompressedLength);
    newStruct->pNext_ = firstBMDJStruct_41c_;
    firstBMDJStruct_41c_ = newStruct;
    return true;
}

bool Zone3D::ProcessAtmosphericEffects()
{
    if (!atmosphericEffects_.IsArchiveLoaded())
        return false;
    atmosphericEffects_.ProcessArchive(&atmosphericEffects_, pAllocator_68_, unknown_ptr_50_);
    for (AtmosphericEffect* effect = atmosphericEffects_.GetFirstEffect();
        effect != NULL; effect = effect->pNext_)
    {
        if (effect->object_.pModel_ == NULL)
            continue;
        NSBXXTex* tex0 = effect->object_.pModel_->GetTEX0();
        if (tex0 == NULL)
            continue;
        textureImageMemory_ += NSBXX_Tex_GetBlock1Length(tex0);
        texturePaletteMemory_ += NSBXX_Tex_GetBlock4Length(tex0);
    }
    return true;
}

void Zone3D::QueueLoadATS_AMBL()
{
    if (isInMainGrottoFloor_23b8_)
    {
        int environ = grotto_.GetActiveGrottoEnviron();
        if (environ == 0)
            environ = 1;
        if (currentGrottoFloor_23ba_ <= 4)
            sprintf(mapListInfo_.maybeModelName, data_020ef19f, environ);
        else if (currentGrottoFloor_23ba_ <= 8)
            sprintf(mapListInfo_.maybeModelName, data_020ef1a9, environ);
        else if (currentGrottoFloor_23ba_ <= 12)
            sprintf(mapListInfo_.maybeModelName, data_020ef1b3, environ);
        else if (currentGrottoFloor_23ba_ <= 16)
            sprintf(mapListInfo_.maybeModelName, data_020ef1bd, environ);
    }
    if (strlen(mapListInfo_.maybeModelName) == 0)
        LoadMapAMDJ();
    else
    {
        BackgroundLoader* loader = BackgroundLoader::GetInstance();
        char filename[40];
        sprintf(filename, data_020ef1c7, data_020ef116, mapListInfo_.maybeModelName[0]);
        atsAMBLLoadHandle_ = loader->QueueLoadFile(filename, NULL);
    }
}

bool Zone3D::UnpackATS_AMBL()
{
    if (atsAMBLLoadHandle_ < 0)
        return true;
    
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    if (loader->GetTaskStatus(atsAMBLLoadHandle_) == 0)
        return false;

    if (loader->GetDetailedTaskStatus(atsAMBLLoadHandle_) != BackgroundLoader::TaskStatus_Complete)
    {
        loader->RemoveTask(atsAMBLLoadHandle_);
        atsAMBLLoadHandle_ = -1;
        LoadMapAMDJ();
        return true;
    }

    void* narcBuffer;
    unsigned int narcLength;
    loader->GetLoadedFileByID(atsAMBLLoadHandle_, &narcBuffer, &narcLength);
    char targetInnerFile[40];
    sprintf(targetInnerFile, data_020ef1d6, mapListInfo_.maybeModelName);

    const void* batsFile;
    unsigned int batsFileLength;

    if (!GetFileInNarc(narcBuffer, targetInnerFile, &batsFile, &batsFileLength, 0))
    {
        loader->RemoveTask(atsAMBLLoadHandle_);
        atsAMBLLoadHandle_ = -1;
        LoadMapAMDJ();
        return true;
    }

    func_02014414(this, batsFile, batsFileLength);
    loader->RemoveTask(atsAMBLLoadHandle_);
    atsAMBLLoadHandle_ = -1;
    LoadMapAMDJ();
    return true;
}

#endif