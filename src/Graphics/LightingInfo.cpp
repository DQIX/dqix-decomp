#include "Graphics/LightingInfo.h"
#include "Resource/Script.h"

static float s_nightLength = 180.0f;
static float s_morningLength = 30.0f;
static float s_dayLength = 180.0f;
static float s_eveningLength = 30.0f;

#define DECLARE_SUM_ARRAY(name, a, b, c, d) static float name[5] = { (name[4] = a + (b + (d + c)), \
    (name[2] = name[3] + d), \
    (name[1] = name[2] + c), \
    /* name[0] = */ name[1] + b) }
DECLARE_SUM_ARRAY(s_dayThresholds, s_eveningLength, s_dayLength, s_morningLength, s_nightLength);

static LightingInfo* s_scriptCurrentLightingInfo;

static int LightingScript_DeclareAdvancedLighting(Script::Parameter* params, int numParams)
{
    if (numParams > 0)
        s_scriptCurrentLightingInfo->gradientCenterOffset_ = params[0].ToFloat();
    s_scriptCurrentLightingInfo->maybeMode_ = 2;
    return 1;
}

static int LightingScript_Nop(Script::Parameter* params, int numParams) { return 1; }

// Never used
static int LightingScript_CreateAdvancedLightingEntry_Alternate(Script::Parameter* params, int numParams)
{
    int index = (params++)->ToInt();

    if (index > 6)
        return 1;

    LightingManager::Light3DConfig secondaryEntry;
    secondaryEntry.maybeEnabled = (params++)->ToInt();
    secondaryEntry.direction[0] = (params++)->ToFloat();
    secondaryEntry.direction[1] = (params++)->ToFloat();
    secondaryEntry.direction[2] = (params++)->ToFloat();
    secondaryEntry.color = (params++)->ToInt();
    s_scriptCurrentLightingInfo->advanced_.light1[index] = secondaryEntry;

    LightingManager::Light3DConfig primaryEntry;
    primaryEntry.maybeEnabled = (params++)->ToInt();
    primaryEntry.direction[0] = 0.5f;
    primaryEntry.direction[1] = -1.0f;
    primaryEntry.direction[2] = 0.5f;
    primaryEntry.color = (params++)->ToInt();
    primaryEntry.maybeEnabled = true;
    s_scriptCurrentLightingInfo->advanced_.light0[index] = primaryEntry;

    int bgColor = (params++)->ToInt();
    int bgColor2 = (params++)->ToInt();
    int ambientColor = (params++)->ToInt();
    int col4 = (params++)->ToInt();
    int spriteColor = (params++)->ToInt();
    int modelColor = (params++)->ToInt();
    int edgeColor = (params++)->ToInt();
    s_scriptCurrentLightingInfo->advanced_.backgroundColor[index] = bgColor;
    s_scriptCurrentLightingInfo->advanced_.backgroundSecondColor[index] = bgColor2;
    s_scriptCurrentLightingInfo->advanced_.maybeAmbientColor[index] = ambientColor;
    s_scriptCurrentLightingInfo->advanced_.unk_142[index] = col4;
    s_scriptCurrentLightingInfo->advanced_.spriteDiffuseColor[index] = spriteColor;
    s_scriptCurrentLightingInfo->advanced_.modelDiffuseColor[index] = modelColor;
    s_scriptCurrentLightingInfo->advanced_.edgeColor[index] = edgeColor;

    return 1;
}

static int LightingScript_DeclareBasicLighting(Script::Parameter* params, int numParams)
{
    if (numParams > 0)
        s_scriptCurrentLightingInfo->gradientCenterOffset_ = params[0].ToFloat();
    s_scriptCurrentLightingInfo->maybeMode_ = 1;
    return 1;
}

static int LightingScript_CreateBasicLightingEntry(Script::Parameter* params, int numParams)
{
    int index = (params++)->ToInt();
    float argX = (params++)->ToFloat();
    float argY = (params++)->ToFloat();
    float argZ = (params++)->ToFloat();
    short bgColor = (params++)->ToInt(); // needs to be signed to get a match
    short bgColor2 = (params++)->ToInt();
    short potBarrelColor = (params++)->ToInt();
    float arg8 = (params++)->ToFloat();
    float arg9 = (params++)->ToFloat();
    short spriteColor = (params++)->ToInt();
    short modelColor = (params++)->ToInt();
    short edgeColor = (params++)->ToInt();

    Vector3float& storageVec = s_scriptCurrentLightingInfo->basic_.unk_0[index];

    storageVec.x = argX;
    storageVec.y = argY;
    storageVec.z = argZ;
    s_scriptCurrentLightingInfo->basic_.backgroundColor[index] = bgColor;
    s_scriptCurrentLightingInfo->basic_.backgroundSecondColor[index] = bgColor2;
    s_scriptCurrentLightingInfo->basic_.potBarrelDiffuseColor[index] = potBarrelColor;
    s_scriptCurrentLightingInfo->basic_.unk_70[index] = arg8;
    s_scriptCurrentLightingInfo->basic_.unk_54[index] = arg9;
    s_scriptCurrentLightingInfo->basic_.unk_e0[index] = true;
    s_scriptCurrentLightingInfo->basic_.spriteDiffuseColor[index] = spriteColor;
    s_scriptCurrentLightingInfo->basic_.modelDiffuseColor[index] = modelColor;
    s_scriptCurrentLightingInfo->basic_.edgeColor[index] = edgeColor;

    return 1;
}

static int LightingScript_CreateFogEntry(Script::Parameter* params, int numParams)
{
    int index = (params++)->ToInt();
    if (index > 6)
        return 1;
    
    LightingManager::FogInfo entry;

    entry.unk_0 = (params++)->ToInt() != 0;
    entry.color = (params++)->ToInt();
    entry.type = (params++)->ToInt();
    entry.depthShift = (params++)->ToInt();
    entry.offset = (params++)->ToInt();
    for (int i = 0; i < 8; i++)
    {
        int packed = (params++)->ToInt();
        entry.densityTable[4 * i] = packed & 0xff;
        entry.densityTable[4 * i + 1] = (packed & 0xff00) >> 8;
        entry.densityTable[4 * i + 2] = (packed & 0xff0000) >> 16;
        entry.densityTable[4 * i + 3] = (packed & 0xff000000) >> 24;
    }
    entry.alpha = (params++)->ToInt();

    LightingInfo* info = s_scriptCurrentLightingInfo;
    info->fogList.entries[index].unk_0 = entry.unk_0;
    info->fogList.entries[index].type = entry.type;
    info->fogList.entries[index].depthShift = entry.depthShift;
    info->fogList.entries[index].offset = entry.offset;
    info->fogList.entries[index].color = entry.color;
    info->fogList.entries[index].alpha = entry.alpha;
    COPY_ARRAY(info->fogList.entries[index].densityTable, entry.densityTable);

    return 1;
}

static int LightingScript_CreateAdvancedLightingEntry(Script::Parameter* params, int numParams)
{
    int index = (params++)->ToInt();
    LightingManager::Light3DConfig light1;
    light1.maybeEnabled = (params++)->ToInt();
    light1.direction[0] = (params++)->ToFloat();
    light1.direction[1] = (params++)->ToFloat();
    light1.direction[2] = (params++)->ToFloat();
    light1.color = (params++)->ToInt();
    s_scriptCurrentLightingInfo->advanced_.light1[index] = light1;

    LightingManager::Light3DConfig light0;
    light0.maybeEnabled = (params++)->ToInt();
    light0.direction[0] = (params++)->ToFloat();
    light0.direction[1] = (params++)->ToFloat();
    light0.direction[2] = (params++)->ToFloat();
    light0.color = (params++)->ToInt();
    s_scriptCurrentLightingInfo->advanced_.light0[index] = light0;

    int bgColor = (params++)->ToInt();
    int bgColor2 = (params++)->ToInt();
    int ambientColor = (params++)->ToInt();
    int col4 = (params++)->ToInt();
    int spriteColor = (params++)->ToInt();
    int modelColor = (params++)->ToInt();
    int edgeColor = (params++)->ToInt();
    s_scriptCurrentLightingInfo->advanced_.backgroundColor[index] = bgColor;
    s_scriptCurrentLightingInfo->advanced_.backgroundSecondColor[index] = bgColor2;
    s_scriptCurrentLightingInfo->advanced_.maybeAmbientColor[index] = ambientColor;
    s_scriptCurrentLightingInfo->advanced_.unk_142[index] = col4;
    s_scriptCurrentLightingInfo->advanced_.spriteDiffuseColor[index] = spriteColor;
    s_scriptCurrentLightingInfo->advanced_.modelDiffuseColor[index] = modelColor;
    s_scriptCurrentLightingInfo->advanced_.edgeColor[index] = edgeColor;

    return 1;
}

static Script::OpcodeLookupEntry s_lightingOpcodes[] = {
    { 0x64, &LightingScript_DeclareAdvancedLighting },
    { 0x65, &LightingScript_Nop },
    { 0x66, &LightingScript_CreateAdvancedLightingEntry_Alternate },
    { 0x67, &LightingScript_DeclareBasicLighting },
    { 0x68, &LightingScript_CreateBasicLightingEntry },
    { 0x69, &LightingScript_CreateFogEntry },
    { 0x6a, &LightingScript_CreateAdvancedLightingEntry },
    { 0, NULL }
};

void ExecuteLightingScript(const void* script, unsigned int length, LightingInfo* manager, SafeAllocator* alloc)
{
    if (length == 0)
        return;
    s_scriptCurrentLightingInfo = manager;
    Script runner;
    runner.Initialize();
    runner.SetOpcodeLookup(s_lightingOpcodes);
    runner.Load(script, length);
    runner.Execute();
    s_scriptCurrentLightingInfo = NULL;
}

void LightingInfo::Initialize()
{
    maybeMode_ = 0;
    gradientCenterOffset_ = 0.0f;
    memset(&basic_, 0, sizeof(basic_));
    memset(&advanced_, 0, sizeof(advanced_));
    memset(&fogList, 0, sizeof(fogList));
}

void LightingInfo::Reset()
{
    maybeMode_ = 0;
    gradientCenterOffset_ = 0.0f;
    memset(&basic_, 0, sizeof(basic_));
    memset(&advanced_, 0, sizeof(advanced_));
    memset(&fogList, 0, sizeof(fogList));
}

void LightingInfo::LoadFromScript(const void* script, unsigned int length, SafeAllocator* alloc)
{
    ExecuteLightingScript(script, length, this, alloc);
}
