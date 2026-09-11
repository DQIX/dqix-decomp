#include "Graphics/AtmosphericEffect.h"
#include "Filesystem/BackgroundLoader.h"
#include "Filesystem/NarcHandle.h"
#include "Filesystem/NitroVM.h"
#include "Filesystem/LowNitroHandle.h"
#include "Filesystem/FileAccessor.h"
#include "Filesystem/FileIO.h"
#include "Resource/Script.h"
#include "Graphics/NSBXX/RenderConfig.h"
#include "Graphics/NSBXX/GeometryFifo.h"
#include "Graphics/NSBXX/NSBXX.h"
#include "GameState.h"
#include "Graphics/LightingManager.h"

// This keeps showing up in multiple translation units: in every case, we have
// four floats in data and then generate five floats in bss in some weird order.
// Note that bss entries are sorted by size, and so this has to be a single array
// in this translation unit at least (because of the size 0xc struct we already
// have coming first). This is truly bizarre: none of these are ever used outside
// of initialization, but they're still here. My best guess is this is created
// by a macro, but we need to see more instances before committing to one form
//
// update: these are the lengths of the various times of day. See LightingManager.cpp
// for an instance where this is actually used. Not sure why it leaks into so many
// files though, maybe it was declared in a header and included in multiple places
#define DECLARE_SUM_ARRAY(a, b, c, d) static float s_sums[5] = { (s_sums[4] = a + (b + (d + c)), \
    (s_sums[2] = s_sums[3] + d), \
    (s_sums[1] = s_sums[2] + c), \
    /* s_sums[0] = */ s_sums[1] + b) }

static float s_dataFloatA = 30.0f;
static float s_dataFloatB = 180.0f;
static float s_dataFloatC = 30.0f;
static float s_dataFloatD = 180.0f;
DECLARE_SUM_ARRAY(s_dataFloatA, s_dataFloatD, s_dataFloatC, s_dataFloatB);

struct EffectScriptData
{
    SafeAllocator* allocator;
    const char* currentObjectName;
    AtmosphericEffectSet* effect;
} static effectScriptData;

#if defined(jpn)
#define func_0200fddc func_0200fc38 
#define func_02033fa0 func_02033ad8 
#define func_0204be20 func_0204cc40 
#define func_0207df90 func_0207ed10 
#define func_0207dfac func_0207ed2c 
#endif

extern "C"
{
    void* func_0200fddc(GameState*);
    void* func_02033fa0(void*);
    bool func_0204be20(void*);

    void func_0207df90(void*);
    void func_0207dfac(void*);
}

void AtmosphericEffect::Initialize()
{
    object_.Initialize();
    name_[0] = '\0';
    scaleFactor_ = 1 << 12;
    currentXOffset_ = 0;
    currentZOffset_ = 0;
    xSpeed_ = 0;
    zSpeed_ = 0;
    originX_ = 0;
    originZ_ = 0;
    flags_ = 0;
    tileWidth_ = 64 << 12;
    tileHeight_ = 64 << 12;
    tileCountX_ = 1;
    tileCountZ_ = 1;
    archiveFileSize_ = 0;
    archiveFileData_ = NULL;
    pNext_ = NULL;
    alpha_ = 31;
}

static int EffectScript_SetScrollSpeed(Script::Parameter* params, int numParams) 
{ 
    if (effectScriptData.effect == NULL)
        return 0;
    if (effectScriptData.currentObjectName == NULL)
        return 0;
    AtmosphericEffect* obj = effectScriptData.effect->GetEffectByName(effectScriptData.currentObjectName);
    if (obj == NULL)
        return 0;

    obj->xSpeed_ = 4096.0f * params[0].ToFloat();
    if (numParams < 2)
    {
        obj->zSpeed_ = obj->xSpeed_;
        return 1;
    }
    else
    {
        obj->zSpeed_ = 4096.0f * params[1].ToFloat();
        return 1;
    }
}

static int EffectScript_SetScrollFlags(Script::Parameter* params, int numParams) 
{
    if (effectScriptData.effect == NULL)
        return 0;
    if (effectScriptData.currentObjectName == NULL)
        return 0;
    AtmosphericEffect* obj = effectScriptData.effect->GetEffectByName(effectScriptData.currentObjectName);
    if (obj == NULL)
        return 0;
    
    int extraFlags = params[0].ToInt();
    if (extraFlags <= 8)
        obj->flags_ |= extraFlags;

    if (numParams >= 2)
    {
        int extraFlags = params[1].ToInt();
        if (extraFlags <= 8)
            obj->flags_ |= extraFlags;
        return 1;
    }
    else
        return 1;

    return 1;
}

static int EffectScript_SelectEffect(Script::Parameter* params, int numParams) 
{
    if (effectScriptData.effect == NULL)
        return 0;
    effectScriptData.currentObjectName = params[0].ToString();
    return 1;
}

static int EffectScript_SetOriginFlags(Script::Parameter* params, int numParams) 
{
    if (effectScriptData.effect == NULL)
        return 0;
    if (effectScriptData.currentObjectName == NULL)
        return 0;
    AtmosphericEffect* obj = effectScriptData.effect->GetEffectByName(effectScriptData.currentObjectName);
    if (obj == NULL)
        return 0;

    int extraFlags = params[0].ToInt();
    if (extraFlags > 8)
        obj->flags_ |= extraFlags;

    if (numParams >= 2)
    {
        int extraFlags = params[1].ToInt();
        if (extraFlags > 8)
            obj->flags_ |= extraFlags;
        return 1;
    }
    else
        return 1;
}

static int EffectScript_SetScrollOrigin(Script::Parameter* params, int numParams) 
{
    if (effectScriptData.effect == NULL)
        return 0;
    if (effectScriptData.currentObjectName == NULL)
        return 0;
    AtmosphericEffect* obj = effectScriptData.effect->GetEffectByName(effectScriptData.currentObjectName);
    if (obj == NULL)
        return 0;

    obj->originX_ = 4096.0f * params[0].ToFloat();
    if (numParams < 2)
    {
        obj->originZ_ = obj->originX_;
        return 1;
    }
    else
    {
        obj->originZ_ = 4096.0f * params[1].ToFloat();
        return 1;
    }
}

static int EffectScript_Duplicate(Script::Parameter* params, int numParams) 
{
    if (effectScriptData.effect == NULL)
        return 0;
    if (numParams < 2)
        return 0;

    const char* sourceName = params[0].ToString();
    const char* newObjName = params[1].ToString();

    AtmosphericEffect* old = effectScriptData.effect->GetEffectByName(sourceName);
    if (old == NULL)
        return 0;

    AtmosphericEffect* newObj =
        (AtmosphericEffect*)effectScriptData.allocator->Allocate(sizeof(AtmosphericEffect));
    newObj->Initialize();
    old->object_.ShallowCloneTo(&newObj->object_);
    newObj->object_.SetInheritedAlpha(31);
    memcpy(newObj->name_, newObjName, sizeof(newObj->name_));
    effectScriptData.effect->AddEffectObject(newObj);
    
    return 1;
}

static int EffectScript_SetScale(Script::Parameter* params, int numParams) 
{
    if (effectScriptData.effect == NULL)
        return 0;
    if (effectScriptData.currentObjectName == NULL)
        return 0;
    AtmosphericEffect* obj = effectScriptData.effect->GetEffectByName(effectScriptData.currentObjectName);
    if (obj == NULL)
        return 0;
    
    obj->scaleFactor_ = 4096.0f * params[0].ToFloat();

    return 1;
}

static int EffectScript_SetAlpha(Script::Parameter* params, int numParams) 
{ 
    if (effectScriptData.effect == NULL)
        return 0;
    if (effectScriptData.currentObjectName == NULL)
        return 0;
    AtmosphericEffect* obj = effectScriptData.effect->GetEffectByName(effectScriptData.currentObjectName);
    if (obj == NULL)
        return 0;
    
    int alpha = params[0].ToInt();
    obj->alpha_ = alpha;
    obj->object_.SetInheritedAlpha(alpha);

    return 1;
}

static int EffectScript_SetTileDimensions(Script::Parameter* params, int numParams) 
{ 
    if (effectScriptData.effect == NULL)
        return 0;
    if (effectScriptData.currentObjectName == NULL)
        return 0;
    AtmosphericEffect* obj = effectScriptData.effect->GetEffectByName(effectScriptData.currentObjectName);
    if (obj == NULL)
        return 0;

    obj->tileWidth_ = 4096.0f * params[0].ToFloat();;
    if (numParams < 2)
        obj->tileHeight_ = obj->tileWidth_;
    else
        obj->tileHeight_ = 4096.0f * params[1].ToFloat();

    return 1;
}

static int EffectScript_SetHiddenTimesOfDay(Script::Parameter* params, int numParams) 
{ 
    unsigned char mask = 0;
    for (int i = 0; i < numParams; i++)
    {
        int extra = (params++)->ToInt();
        mask |= extra;
    }
    effectScriptData.effect->timeOfDayHideBits_ = mask;
    return 1;
}

static Script::OpcodeLookupEntry effectScriptOpcodes[] = {
    { 0x64, &EffectScript_SetScrollSpeed },
    { 0x65, &EffectScript_SetScrollFlags },
    { 0x66, &EffectScript_SelectEffect },
    { 0x67, &EffectScript_SetOriginFlags },
    { 0x68, &EffectScript_SetScrollOrigin },
    { 0x69, &EffectScript_Duplicate },
    { 0x6a, &EffectScript_SetScale },
    { 0x6b, &EffectScript_SetAlpha },
    { 0x6c, &EffectScript_SetTileDimensions },
    { 0x6d, &EffectScript_SetHiddenTimesOfDay },
    { 0, NULL }};

void AtmosphericEffectSet::Reset()
{
    firstThing_ = NULL;
    archiveLoadHandle_ = -1;
    unknown_10_ = 2;
    rawArchive_ = NULL;
    archiveLength_ = 0;
    effectFlags_ = 0;
    alpha_ = 31;
    fadeTimeRemaining_ = 0;
    timeOfDayIndex_ = 0;
    unknown_14_bit_2_ = 0;
    timeOfDayHideBits_ = 0;
    visibilityDirty_ = true;
}

void AtmosphericEffectSet::LoadArchive(const char* name)
{
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    char mseFilename[80];
    sprintf(mseFilename, "data/map/%s.mse", name);
    archiveLoadHandle_ = loader->QueueLoadFile(mseFilename, NULL);
    unknown_10_ = 1;
    rawArchive_ = NULL;
    archiveLength_ = 0;
    alpha_ = 31;
    fadeTimeRemaining_ = 0;
    timeOfDayIndex_ = 0;
    unknown_14_bit_2_ = 0;
    timeOfDayHideBits_ = 0;
    visibilityDirty_ = true;
}

bool AtmosphericEffectSet::IsArchiveLoaded()
{
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    if (unknown_10_ == 1)
    {
        if (loader->GetTaskStatus(archiveLoadHandle_) != 0)
        {
            if (loader->GetDetailedTaskStatus(archiveLoadHandle_) != BackgroundLoader::TaskStatus_Complete)
            {
                loader->RemoveTask(archiveLoadHandle_);
                archiveLoadHandle_ = -1;
                unknown_10_ = 2;
                return true;
            }
            loader->GetLoadedFileByID(archiveLoadHandle_, &rawArchive_, &archiveLength_);
            unknown_10_ = 2;
        }
        return false;
    }
    else if (unknown_10_ == 2)
        return true;
    return false;
}

void AtmosphericEffectSet::ProcessArchive(AtmosphericEffectSet *target, SafeAllocator *allocator, void *pZone50Thing)
{
    if (rawArchive_ == NULL)
        return;

    AtmosphericEffect* lastObject = NULL;
    
    const void* bmedFileData = NULL;
    unsigned int bmedFileLength = 0;
    bool bmedFound = false;
    effectScriptData.currentObjectName = NULL;

    AtmosphericEffect* loopObject;
    unsigned int fileID;
    
    effectScriptData.allocator = allocator;
    effectScriptData.effect = target;

    NarcHandle narc;
    if (narc.Initialize("ARC", (const unsigned char*)rawArchive_))
    {
        NitroVM vm;
        fileID = 0;
        NitroVM_Initialize(&vm);
        while (PrepareReadFileInNARCByID(&vm, &narc, fileID))
        {
            char innerFilePath[80];
            NitroVM_WriteOutFilePath(&vm, innerFilePath, sizeof(innerFilePath));
            const char* extension = strrchr(innerFilePath, '.');
            if (extension == NULL)
            {
                NitroVM_FinishRead(&vm);
                fileID++;
                continue;
            }
            unsigned int innerFileLength = vm.fileInfo.endOffset - vm.fileInfo.startOffset;
            NitroVM_FinishRead(&vm);
            const void* innerFile = narc.GetFileByIndex(fileID);
            if (strcmp(".cmed", extension) == 0)
            {
                AtmosphericEffect* newObj = (AtmosphericEffect*)allocator->Allocate(sizeof(AtmosphericEffect));
                newObj->Initialize();
                if (firstThing_ == NULL)
                {
                    lastObject = firstThing_ = newObj;
                    lastObject->archiveFileSize_ = innerFileLength;
                    lastObject->archiveFileData_ = innerFile;
                }
                else
                {
                    lastObject = lastObject->pNext_ = newObj;
                    lastObject->archiveFileSize_ = innerFileLength;
                    lastObject->archiveFileData_ = innerFile;
                    
                }
            }
            else if (strcmp(".bmed", extension) == 0)
            {
                bmedFound = true;
                bmedFileData = innerFile;
                bmedFileLength = innerFileLength;
                
            }
            fileID++;
        }
        narc.Destroy();
    }

    if (lastObject == NULL)
        return;

    for (loopObject = firstThing_; loopObject != NULL; loopObject = loopObject->pNext_)
    {
        if (narc.Initialize("ARC", (const unsigned char*)loopObject->archiveFileData_))
        {
            fileID = 0;
            NitroVM vm;
            NitroVM_Initialize(&vm);
            ObjectArchiveLoadInfo loadInfo;
            loadInfo.unk_0 = 0;
            loadInfo.fileData = NULL;
            loadInfo.unk_8 = 0;
            loadInfo.allocator = NULL;
            loadInfo.unk_10 = 0;
            loadInfo.unk_14 = 0;
            loadInfo.unk_18 = 0;
            loadInfo.packageID = 0;
            while (PrepareReadFileInNARCByID(&vm, &narc, fileID))
            {
                char innerFilePath[80];
                NitroVM_WriteOutFilePath(&vm, innerFilePath, sizeof(innerFilePath));
                const char* extension = strrchr(innerFilePath, '.');
                const char* filename = strrchr(innerFilePath, '/');
                if (extension == NULL)
                {
                    NitroVM_FinishRead(&vm);
                    fileID++;
                    continue;
                }
                unsigned int innerFileLength = vm.fileInfo.endOffset - vm.fileInfo.startOffset;
                NitroVM_FinishRead(&vm);
                const void* innerFile = narc.GetFileByIndex(fileID);
                if (strcmp(".chr", extension) == 0)
                {
                    loopObject->archiveFileData_ = innerFile;
                    loopObject->archiveFileSize_ = innerFileLength;
                    memcpy(loopObject->name_, filename + 1, sizeof(loopObject->name_));
                }
                fileID++;
            }
            narc.Destroy();
            func_0207df90(pZone50Thing);
            loadInfo.allocator = allocator;
            loadInfo.fileData = loopObject->archiveFileData_;
            loadInfo.unk_8 = loopObject->archiveFileSize_;
            loadInfo.unk_10 = 1;
            loopObject->object_.LoadFromCHRArchive(&loadInfo);
            func_0207dfac(pZone50Thing);
        }
    }

    if (bmedFound && firstThing_ != NULL)
    {
        Script runner;
        runner.Initialize();
        runner.SetOpcodeLookup(effectScriptOpcodes);
        runner.Load(bmedFileData, bmedFileLength);
        runner.Execute();
        for (loopObject = firstThing_; loopObject != NULL;)
        {
            if (loopObject->scaleFactor_ <= 0)
            {
                loopObject = loopObject->pNext_;
                continue;
            }
            // treat the scale factor as shrinking the screen instead of enlarging the tiles
            fix32_t effectiveScreenWidth = fix32_Divide(256 << 12, loopObject->scaleFactor_);
            fix32_t effectiveScreenHeight = fix32_Divide(192 << 12, loopObject->scaleFactor_);
            if (!(loopObject->flags_ & (1 << 6)))
            {
                if (loopObject->flags_ & (1 << 7))
                    loopObject->originX_ = -loopObject->originX_;
                else
                    loopObject->originX_ = 0;
            }
            if (!(loopObject->flags_ & (1 << 4)))
            {
                if (loopObject->flags_ & (1 << 5))
                    loopObject->originZ_ = -loopObject->originZ_;
                else
                    loopObject->originZ_ = 0;
            }
            unsigned int floorNumTilesX = fix32_Divide(effectiveScreenWidth, loopObject->tileWidth_) / 4096.0f;
            loopObject->tileCountX_ = floorNumTilesX + 1;
            unsigned int floorNumTilesZ = fix32_Divide(effectiveScreenHeight, loopObject->tileHeight_) / 4096.0f;
            loopObject->tileCountZ_ = floorNumTilesZ + 1;

            fix32_t oldE0 = loopObject->originX_;
            fix32_t xpercent = fix32_Divide(effectiveScreenWidth, 100 << 12);
            loopObject->originX_ = FIX32_MULTIPLY(xpercent, oldE0);
            fix32_t oldE4 = loopObject->originZ_;
            fix32_t ypercent = fix32_Divide(effectiveScreenHeight, 100 << 12);
            loopObject->originZ_ = FIX32_MULTIPLY(ypercent, oldE4);

            fix32_t oldD8 = loopObject->xSpeed_;
            fix32_t ratioE8 = fix32_Divide(effectiveScreenWidth, loopObject->tileWidth_);
            loopObject->xSpeed_ = FIX32_MULTIPLY(ratioE8, oldD8);
            fix32_t oldDC = loopObject->zSpeed_;
            fix32_t ratioEC = fix32_Divide(effectiveScreenHeight, loopObject->tileHeight_);
            loopObject->zSpeed_ = FIX32_MULTIPLY(ratioEC, oldDC);

            loopObject = loopObject->pNext_;
        }
    }
    BackgroundLoader::GetInstance()->RemoveTask(archiveLoadHandle_);
    archiveLoadHandle_ = -1;
    rawArchive_ = NULL;
    archiveLength_ = 0;
}

bool AtmosphericEffectSet::Draw()
{
    if (firstThing_ == NULL)
        return false;
    if (effectFlags_ & 1)
        return false;

    DetermineVisibilityFromTimeOfDay();
    DetermineVisibilityFromUnknown();
    AdvanceAlphaFade();

    Vector3fix eye;
    Vector3fix target;
    Vector3fix up;

    up.x = 0;
    up.y = 0;
    up.z = 1 << 12;

    eye.x = 0;
    eye.y = 10 << 12;
    eye.z = 0;

    target.x = 0;
    target.y = 0;
    target.z = 0;

    int polygonID = 55; // max is 63
    for (AtmosphericEffect* loopObject = firstThing_; loopObject != NULL;)
    {
        if (loopObject->scaleFactor_ <= 0)
        {
            loopObject = loopObject->pNext_;
            continue;
        }

        // treat the scale factor as shrinking the screen instead of enlarging the tiles
        fix32_t effectiveScreenWidth = fix32_Divide(256 << 12, loopObject->scaleFactor_);
        fix32_t effectiveScreenHeight = fix32_Divide(192 << 12, loopObject->scaleFactor_);

        Mat4x4_WriteProjectionUnknown(
            effectiveScreenHeight >> 1, -effectiveScreenHeight >> 1,
            -effectiveScreenWidth >> 1, effectiveScreenWidth >> 1,
            1 << 12, 100 << 12, 1 << 12, &data_0210a010.projectionMatrix);

        data_0210a010.flags &= ~((1 << RENDER_CONFIG_FLAG_4) | (1 << RENDER_CONFIG_FLAG_6));
        data_0210a010.eyeVector = eye;
        data_0210a010.upVector = up;
        data_0210a010.targetVector = target;
        Mat4x3_WriteViewMatrix(&eye, &up, &target, &data_0210a010.viewMatrix);
        
        data_0210a010.flags &= ~(
            (1 << RENDER_CONFIG_FLAG_INVERSE_VIEW_CACHE_VALID) |
            (1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID) |
            (1 << RENDER_CONFIG_FLAG_5) | (1 << RENDER_CONFIG_FLAG_6));
        
        int bVar2 = false;
        int bVar3 = false;

        int centeringOffsetX = (effectiveScreenWidth - loopObject->tileWidth_) / 2;
        if (centeringOffsetX < 0)
            centeringOffsetX = 0;

        int centeringOffsetZ = (effectiveScreenHeight - loopObject->tileHeight_) / 2;
        if (centeringOffsetZ < 0)
            centeringOffsetZ = 0;

        int xflip = 1;
        int zflip = 1;

        if (loopObject->flags_ & 1)
            zflip = -1;
        if (loopObject->flags_ & 4)
            xflip = -1;

        int numXDraws = loopObject->tileCountX_;
        int numZDraws = loopObject->tileCountZ_;

        if ((loopObject->flags_ & 4) || (loopObject->flags_ & 8))
        {
            numXDraws++;
            bVar2 = true;
        }
        if ((loopObject->flags_ & 1) || (loopObject->flags_ & 2))
        {
            numZDraws++;
            bVar3 = true;
        }
        for (int loopX = 0; loopX < numXDraws; loopX++)
        {
            for (int loopZ = 0; loopZ < numZDraws; loopZ++)
            {
                fix32_t zfoo = loopObject->originZ_ + (loopZ * loopObject->tileHeight_ - centeringOffsetZ);
                fix32_t xfoo = loopObject->originX_ + (loopX * loopObject->tileWidth_ - centeringOffsetX);
             
                // this might look like xflip = 1 means things move left, but it doesn't,
                // this is because the view matrix has (0, 0, 1) as up and so world
                // coordinates (x, y, z) transform proportional to (-x, z, y) on screen
                fix32_t finalZ = zflip * (zfoo - loopObject->currentZOffset_);
                fix32_t finalX = xflip * (xfoo - loopObject->currentXOffset_);
                
                loopObject->object_.position_.x = finalX;
                loopObject->object_.position_.y = -45 << 12;
                loopObject->object_.position_.z = finalZ;
                RenderConfig::SubmitToFifo();
                SendQueuedDataToGeometryFifo();
                // clear the polygon ID from the POLYGON_ATTR bitmask
                NSBXX_Model_AdjustPolygonAttrMask(loopObject->object_.pModel_->rawInternalModel_, false, 0x3f000000);
                RenderConfig::SetPolygonAttributes(0, 0, 2 | 1, polygonID, 31, 0);
                loopObject->object_.Draw(true);
            }
        }
        polygonID++;
        if (bVar2)
            loopObject->currentXOffset_ += loopObject->xSpeed_;
        if (bVar3)
            loopObject->currentZOffset_ += loopObject->zSpeed_;

        if (loopObject->currentXOffset_ >= loopObject->tileWidth_)
            loopObject->currentXOffset_ -= loopObject->tileWidth_;

        if (loopObject->currentZOffset_ >= loopObject->tileHeight_)
            loopObject->currentZOffset_ -= loopObject->tileHeight_;

        loopObject = loopObject->pNext_;
    }

    RenderConfig::SubmitToFifo();
    SendQueuedDataToGeometryFifo();
    return true;
}

void AtmosphericEffectSet::AddEffectObject(AtmosphericEffect *obj)
{
    if (firstThing_ == NULL)
    {
        firstThing_ = obj;
        return;
    }
    AtmosphericEffect* listEnd = firstThing_;
    while (listEnd->pNext_ != NULL)
        listEnd = listEnd->pNext_;
    listEnd->pNext_ = obj;
}

AtmosphericEffect* AtmosphericEffectSet::GetEffectByName(const char *name)
{
    if (name == NULL)
        return NULL;

    for (AtmosphericEffect* loopObject = firstThing_; loopObject != NULL; loopObject = loopObject->pNext_)
    {
        if (strcmp(name, loopObject->name_) == 0)
            return loopObject;
    }
    return NULL;
}

AtmosphericEffect* AtmosphericEffectSet::GetFirstEffect() { return firstThing_; }

void AtmosphericEffectSet::AdvanceAlphaFade()
{
    if (!(effectFlags_ & (1 << 1)) && !(effectFlags_ & (1 << 2)))
        return;

    fadeTimeRemaining_--;
    if (effectFlags_ & 2)
    {
        alpha_ = 31 * ((90 - fadeTimeRemaining_) / 90.0f);
    }   
    else if (effectFlags_ & 4)
    {
        alpha_ = 31 * (fadeTimeRemaining_ / 90.0f);
    }

    for (AtmosphericEffect* loopObject = firstThing_; loopObject != NULL; loopObject = loopObject->pNext_)
    {
        float alphaScaleFactor = alpha_ / 31.0f;
        loopObject->object_.SetInheritedAlpha(loopObject->alpha_ * alphaScaleFactor);
    }

    if (fadeTimeRemaining_ == 0)
    {
        fadeTimeRemaining_ = 0;
        effectFlags_ &= ~((1 << 1) | (1 << 2));
    }
}

void AtmosphericEffectSet::StartFadeOut()
{
    float alphaScaleFactor = alpha_ / 31.0f;
    fadeTimeRemaining_ = 90 * alphaScaleFactor;
    if (fadeTimeRemaining_ != 0)
        effectFlags_ = (effectFlags_ | (1 << 2)) & ~(1 << 1);
}

void AtmosphericEffectSet::StartFadeIn()
{
    float missingAlphaScaleFactor = (31 - alpha_) / 31.0f;
    fadeTimeRemaining_ = 90 * missingAlphaScaleFactor;
    if (fadeTimeRemaining_ != 0)
        effectFlags_ = (effectFlags_ | (1 << 1)) & ~(1 << 2);
}

void AtmosphericEffectSet::DetermineVisibilityFromTimeOfDay()
{
    if (timeOfDayHideBits_ == 0)
        return;

    int inputTimeOfDay = LightingManager::GetInstance()->timeOfDayIndex_;

    if (inputTimeOfDay != timeOfDayIndex_ || visibilityDirty_)
    {
        if (timeOfDayHideBits_ & (1 << inputTimeOfDay))
            StartFadeOut();
        else
            StartFadeIn();
        if (visibilityDirty_)
        {
            fadeTimeRemaining_ = 1;
            visibilityDirty_ = false;
        }
    }
    timeOfDayIndex_ = inputTimeOfDay;
}

void AtmosphericEffectSet::DetermineVisibilityFromUnknown()
{
    bool inputShouldHide = func_0204be20(func_02033fa0(func_0200fddc(GameState::GetInstance())));

    if (inputShouldHide != unknown_14_bit_2_ || visibilityDirty_)
    {
        if (inputShouldHide)
            StartFadeOut();
        else
            StartFadeIn();
        if (visibilityDirty_)
        {
            fadeTimeRemaining_ = 1;
            visibilityDirty_ = false;
        }
    }
    unknown_14_bit_2_ = inputShouldHide;
}