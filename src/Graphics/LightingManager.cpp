#include "Graphics/LightingManager.h"
#include "World/Zone3D.h"
#include "System/Memory.h"
#include "System/Graphics.h"
#include "Combat/Main/BattleList.h"
#include "Grotto/Overlay_17/Struct44C8.h"

#if defined(jpn)
#define func_020100bc func_0200ff18
#define func_02010208 func_02010064
#define func_02010288 func_020100e4
#define func_0202ec84 func_0202e7f4
#define func_0205ec34 func_0205ff20
#define func_0206dfb0 func_0206f104
#define func_0207ba28 func_0207c860
#define func_020c54a4 func_020c6f70
#define func_020c555c func_020c7028
#define func_020c5574 func_020c7040

#define func_ov000_02160fd4 func_ov000_02162740

#define func_ov017_021901ac func_ov017_02190d90
#define func_ov017_021b8468 func_ov017_021b8978
#define func_ov017_021b8478 func_ov017_021b8988
#endif

struct FifoCommandInfo
{
    unsigned char numOperands;
    bool hasColor;
} static s_commandColorInfo[8][16] = {
    { // Commands 0x00-0x0F: no operands, no color
        0
    },
    { // Commands 0x10-0x1c
        { 1, false }, { 0, false }, { 1, false }, { 1, false },
        { 1, false }, { 0, false }, { 16, false }, { 12, false },
        { 16, false }, { 12, false }, { 9, false }, { 3, false },
        { 3, false },
    },
    { // Commands 0x20-0x2b
        { 1, true }, { 1, false }, { 1, false }, { 2, false },
        { 1, false }, { 1, false }, { 1, false }, { 1, false },
        { 1, false }, { 1, false }, { 1, false }, { 1, false },
    },
    { // Commands 0x30-0x34
        { 1, false }, { 1, false }, { 1, false }, { 1, false },
        { 32, false },
    },
    { // Command 0x40
        { 1, false },
    },
    { // Command 0x50
        { 1, false },
    },
    { // Command 0x60
        { 1, false },
    },
    { // Commands 0x70-0x72
        { 3, false }, { 2, false }, { 1, false },
    }
};

static float s_dayTransitionLength = 2.0f;
static float s_nightLength = 180.0f;
static float s_morningLength = 30.0f;
static float s_dayLength = 180.0f;
static float s_eveningLength = 30.0f;

#define DECLARE_SUM_ARRAY(name, a, b, c, d) static float name[5] = { (name[4] = a + (b + (d + c)), \
    (name[2] = name[3] + d), \
    (name[1] = name[2] + c), \
    /* name[0] = */ name[1] + b) }
DECLARE_SUM_ARRAY(s_dayThresholds, s_eveningLength, s_dayLength, s_morningLength, s_nightLength);
static LightingManager s_lightingManager;

extern "C"
{
    // camera data?
    void* func_020100bc(BattleStruct*);
    // one of various deltaTime counters
    int func_02010208(BattleStruct*);
    // set day/night time
    void func_02010288(BattleStruct*, float);

    void func_0202ec84(void*, const Vector3fix*, int*, int*);

    bool func_0206dfb0(void*, void*, int);

    char* func_0205ec34(); 

    void func_0207ba28(LightingInfo*, int, float*, float*, float*);

    void func_020c54a4(int, int, int, int);
    // set edge colors
    void func_020c555c(unsigned short*);
    // set fog table
    void func_020c5574(void*);

    // do bitmask of something
    int func_ov000_02160fd4(void*, int);

    void* func_ov017_021b8468(void*);
    void func_ov017_021901ac(Struct_ov017_44C8*);
    void* func_ov017_021b8478(void*);
}

void SetVector3fixComponents(Vector3fix*, fix32_t, fix32_t, fix32_t);

LightingManager* LightingManager::GetInstance() { return &s_lightingManager; }

void GetDayThresholds(float *outNightToMorning, float *outMorningToDay,
    float *outDayToEvening, float *outEveningToNight, float *outTransitionTime)
{
    *outNightToMorning = s_dayThresholds[2];
    *outMorningToDay = s_dayThresholds[1];
    *outDayToEvening = s_dayThresholds[0];
    *outEveningToNight = s_dayThresholds[3];
    *outTransitionTime = s_dayTransitionLength;
}

void LightingManager::AdvancedLighting::FillMissingEntries()
{
    // First pass: handle light 0 and colors
    for (int index = 0; index < 4; index++)
    {
        if (light0[index].maybeEnabled == true)
            continue;
        
        int sourceIndex = index;
        for (int tryCounter = 0; tryCounter < 3; tryCounter++)
        {
            sourceIndex--;
            if (sourceIndex < 0)
                sourceIndex = 3;
            if (light0[sourceIndex].maybeEnabled == true)
                break;
            if (tryCounter == 3)
            {
                for (int i = 0; i < 4; i++)
                {
                    light0[i].color = 0x7fff;
                    light0[i].direction[0] = 0.0f;
                    light0[i].direction[1] = -1.0f;
                    light0[i].direction[2] = 0.0f;
                    maybeAmbientColor[i] = backgroundColor[i] = backgroundSecondColor[i] = 0;
                    unk_142[i] = spriteDiffuseColor[i] = modelDiffuseColor[i] = 0x7fff;
                    edgeColor[i] = 0x1086;
                }
                return;
            }
        }
        light0[index].color = light0[sourceIndex].color;
        light0[index].direction[0] = light0[sourceIndex].direction[0];
        light0[index].direction[1] = light0[sourceIndex].direction[1];
        light0[index].direction[2] = light0[sourceIndex].direction[2];
        maybeAmbientColor[index] = maybeAmbientColor[sourceIndex];
        backgroundColor[index] = backgroundColor[sourceIndex];
        backgroundSecondColor[index] = backgroundSecondColor[sourceIndex];
        unk_142[index] = unk_142[sourceIndex];
        spriteDiffuseColor[index] = spriteDiffuseColor[sourceIndex];
        modelDiffuseColor[index] = modelDiffuseColor[sourceIndex];
        edgeColor[index] = edgeColor[sourceIndex];        
    }

    // Second pass for light 1 specifically
    for (int index = 0; index < 4; index++)
    {
        if (light1[index].maybeEnabled == true)
            continue;

        int sourceIndex = index;
        for (int tryCounter = 0; tryCounter < 3; tryCounter++)
        {
            sourceIndex--;
            if (sourceIndex < 0)
                sourceIndex = 3;
            if (light1[sourceIndex].maybeEnabled == true)
                break;
            if (tryCounter == 3)
            {
                for (int i = 0; i < 4; i++)
                {
                    light1[i].color = 0x7fff;
                    light1[i].direction[0] = 0.0f;
                    light1[i].direction[1] = -1.0f;
                    light1[i].direction[2] = 0.0f;
                }
                return;
            }
        }
        light1[index].color = light1[sourceIndex].color;
        light1[index].direction[0] = light1[sourceIndex].direction[0];
        light1[index].direction[1] = light1[sourceIndex].direction[1];
        light1[index].direction[2] = light1[sourceIndex].direction[2];
    }
}

void LightingManager::BasicLighting::FillMissingEntries()
{
    for (int index = 0; index < 4; index++)
    {
        if (unk_e0[index] == 1)
            continue;

        int sourceIndex = index;
        for (int tryCounter = 0; tryCounter < 3; tryCounter++)
        {
            sourceIndex--;
            if (sourceIndex < 0)
                sourceIndex = 3;
            if (unk_e0[sourceIndex] == 1)
                break;
            // nice try but this never runs, the loop stops before this
            if (tryCounter == 3)
            {
                for (int i = 0; i < 4; i++)
                {
                    unk_0[i].x = 1.0f;
                    unk_0[i].y = 1.0f;
                    unk_0[i].z = 1.0f;
                    unk_54[i] = 0.0f;
                    unk_70[i] = 0.0f;
                    backgroundColor[i] = 0;
                    backgroundSecondColor[i] = 0;
                    potBarrelDiffuseColor[i] = 0x7fff;
                    spriteDiffuseColor[i] = 0x7fff;
                    modelDiffuseColor[i] = 0x7fff;
                    edgeColor[i] = 0x1086;
                }
                return;
            }
        }

        unk_0[index].x = unk_0[sourceIndex].x;
        unk_0[index].y = unk_0[sourceIndex].y;
        unk_0[index].z = unk_0[sourceIndex].z;
        unk_54[index] = unk_54[sourceIndex];
        unk_70[index] = unk_70[sourceIndex];
        backgroundColor[index] = backgroundColor[sourceIndex];
        backgroundSecondColor[index] = backgroundSecondColor[sourceIndex];
        potBarrelDiffuseColor[index] = potBarrelDiffuseColor[sourceIndex];
        spriteDiffuseColor[index] = spriteDiffuseColor[sourceIndex];
        modelDiffuseColor[index] = modelDiffuseColor[sourceIndex];
        edgeColor[index] = edgeColor[sourceIndex];
    }
}

void LightingManager::FogList::FillMissingEntries()
{
    for (int index = 0; index < 4; index++)
    {
        if (entries[index].unk_0 == 1)
            continue;

        int sourceIndex = index;
        for (int tryCounter = 0; tryCounter < 3; tryCounter++)
        {
            sourceIndex--;
            if (sourceIndex < 0)
                sourceIndex = 3;
            if (entries[sourceIndex].unk_0 == 1)
                break;
            // nice try but this never runs, the loop stops before this
            if (tryCounter == 3)
            {
                for (int i = 0; i < 4; i++)
                    for (int j = 0; j < 32; j++)
                        entries[i].densityTable[j] = 0;
                return;
            }
        }
        entries[index].type = entries[sourceIndex].type;
        entries[index].depthShift = entries[sourceIndex].depthShift;
        entries[index].offset = entries[sourceIndex].offset;
        entries[index].color = entries[sourceIndex].color;
        entries[index].alpha = entries[sourceIndex].alpha;
        for (int i = 0; i < 32; i++)
            entries[index].densityTable[i] = entries[sourceIndex].densityTable[i];
    }
}

bool LightingManager::GetFifoCommandData(int opcode, int* outNumArgs)
{
    int high = opcode >> 4; // 0 to 7
    int low = opcode ^ (high << 4); // 0 to 15
    *outNumArgs = s_commandColorInfo[high][low].numOperands;
    return s_commandColorInfo[high][low].hasColor;
}

unsigned short LightingManager::ColorTransformTintBrightnessContrast(unsigned int inColor)
{
    (void)GetBattleStruct();
    
    int inRed = inColor & 0x1f;
    int inGreen = (inColor & 0x3e0) >> 5;
    int inBlue = (inColor & 0x7c00) >> 10;

    inRed *= redMix_2c_;
    if (inRed > 31)
        inRed = 31;
    inGreen *= greenMix_30_;
    if (inGreen > 31)
        inGreen = 31;
    inBlue *= blueMix_34_;
    if (inBlue > 31)
        inBlue = 31;

    float normalizedRed = (inRed / 31.0f) + mixBrightness_;
    float normalizedGreen = (inGreen / 31.0f) + mixBrightness_;
    float normalizedBlue = (inBlue / 31.0f) + mixBrightness_;

    if (normalizedRed < 0.0f)
        normalizedRed = 0.0f;
    if (normalizedRed > 1.0f)
        normalizedRed = 1.0f;
    
    if (normalizedGreen < 0.0f)
        normalizedGreen = 0.0f;
    if (normalizedGreen > 1.0f)
        normalizedGreen = 1.0f;
    
    if (normalizedBlue < 0.0f)
        normalizedBlue = 0.0f;
    if (normalizedBlue > 1.0f)
        normalizedBlue = 1.0f;

    if (normalizedRed > 0.5f)
    {
        normalizedRed += mixContrast_;
        if (normalizedRed < 0.5f)
            normalizedRed = 0.5f;
    }
    else
    {
        normalizedRed -= mixContrast_;
        if (normalizedRed > 0.5f)
            normalizedRed = 0.5f;
    }

    if (normalizedGreen > 0.5f)
    {
        normalizedGreen += mixContrast_;
        if (normalizedGreen < 0.5f)
            normalizedGreen = 0.5f;
    }
    else
    {
        normalizedGreen -= mixContrast_;
        if (normalizedGreen > 0.5f)
            normalizedGreen = 0.5f;
    }

    if (normalizedBlue > 0.5f)
    {
        normalizedBlue += mixContrast_;
        if (normalizedBlue < 0.5f)
            normalizedBlue = 0.5f;
    }
    else
    {
        normalizedBlue -= mixContrast_;
        if (normalizedBlue > 0.5f)
            normalizedBlue = 0.5f;
    }

    if (normalizedRed < 0.0f)
        normalizedRed = 0.0f;
    if (normalizedRed > 1.0f)
        normalizedRed = 1.0f;
    
    if (normalizedGreen < 0.0f)
        normalizedGreen = 0.0f;
    if (normalizedGreen > 1.0f)
        normalizedGreen = 1.0f;
    
    if (normalizedBlue < 0.0f)
        normalizedBlue = 0.0f;
    if (normalizedBlue > 1.0f)
        normalizedBlue = 1.0f;

    int endRed = 31.0f * normalizedRed;
    int endGreen = 31.0f * normalizedGreen;
    int endBlue = 31.0f * normalizedBlue;
    return (endRed | (endGreen << 5) | (endBlue << 10));
}

void LightingManager::GetCurrentAdvancedLightingValues(unsigned short *outMaybeAmbient,
    unsigned short *outBGColor, unsigned short *outBGSecondColor, unsigned short *outArg4,
    unsigned short *outSpriteDiffuse, unsigned short *outModelDiffuse, unsigned short *outEdgeColor,
    Light3DConfig *outLight0, Light3DConfig *outLight1)
{
    if (pZone_ == NULL)
        return;
    int currentTimeType;
    int nextTimeType;
    float transitionTime;
    float lerpFactor;
    LightingInfo* lightingInfo = &pZone_->lighting_;
    if (lightingInfo->maybeMode_ != 2)
        return;

    if (lightingIndexOverride_ != 0)
    {
        *outMaybeAmbient = lightingInfo->advanced_.maybeAmbientColor[lightingIndexOverride_];
        *outBGColor = lightingInfo->advanced_.backgroundColor[lightingIndexOverride_];
        *outBGSecondColor = lightingInfo->advanced_.backgroundSecondColor[lightingIndexOverride_];
        *outArg4 = lightingInfo->advanced_.unk_142[lightingIndexOverride_];
        *outSpriteDiffuse = lightingInfo->advanced_.spriteDiffuseColor[lightingIndexOverride_];
        *outModelDiffuse = lightingInfo->advanced_.modelDiffuseColor[lightingIndexOverride_];
        *outEdgeColor = lightingInfo->advanced_.edgeColor[lightingIndexOverride_];
        *outLight0 = lightingInfo->advanced_.light0[lightingIndexOverride_];
        *outLight1 = lightingInfo->advanced_.light1[lightingIndexOverride_];
    }
    else
    {
        (void)GetBattleStruct();
        currentTimeType = timeOfDayIndex_;
        float endTimes[4] = {
            s_dayThresholds[2], s_dayThresholds[1], s_dayThresholds[0], s_dayThresholds[4]
        };
        float currentTimeValue = dayNightTimer_;
        nextTimeType = currentTimeType + 1;
        if (nextTimeType > 3)
            nextTimeType = 0;
        transitionTime = (currentTimeValue - (endTimes[currentTimeType] - s_dayTransitionLength));
        if (transitionTime < 0.0f)
            lerpFactor = 0.0f;
        else
        {
            lerpFactor = transitionTime;
            lerpFactor /= s_dayTransitionLength;
        }

        void* ov17thing = func_ov017_0218b5b0()->unknown_ptr_3718;
        if (ov17thing != NULL)
        {
            void* ov0thing = func_ov017_021b8468(ov17thing);
            if (ov0thing != NULL && func_ov000_02160fd4(ov0thing, 1 << 9))
            {
                unsigned char* overrideData = (unsigned char*)func_ov017_021b8478(ov17thing);
                currentTimeType = overrideData[5];
                lerpFactor = 0.0f;
                nextTimeType = currentTimeType + 1;
                if (nextTimeType > 3)
                    nextTimeType = 0;
            }
        }

        *outMaybeAmbient = InterpolateColors(
            lightingInfo->advanced_.maybeAmbientColor[currentTimeType],
            lightingInfo->advanced_.maybeAmbientColor[nextTimeType],
            lerpFactor);

        *outBGColor = InterpolateColors(
            lightingInfo->advanced_.backgroundColor[currentTimeType],
            lightingInfo->advanced_.backgroundColor[nextTimeType],
            lerpFactor);

        *outBGSecondColor = InterpolateColors(
            lightingInfo->advanced_.backgroundSecondColor[currentTimeType],
            lightingInfo->advanced_.backgroundSecondColor[nextTimeType],
            lerpFactor);

        *outArg4 = InterpolateColors(
            lightingInfo->advanced_.unk_142[currentTimeType],
            lightingInfo->advanced_.unk_142[nextTimeType],
            lerpFactor);

        *outSpriteDiffuse = InterpolateColors(
            lightingInfo->advanced_.spriteDiffuseColor[currentTimeType],
            lightingInfo->advanced_.spriteDiffuseColor[nextTimeType],
            lerpFactor);

        *outModelDiffuse = InterpolateColors(
            lightingInfo->advanced_.modelDiffuseColor[currentTimeType],
            lightingInfo->advanced_.modelDiffuseColor[nextTimeType],
            lerpFactor);

        *outEdgeColor = InterpolateColors(
            lightingInfo->advanced_.edgeColor[currentTimeType],
            lightingInfo->advanced_.edgeColor[nextTimeType],
            lerpFactor);

        outLight0->color = InterpolateColors(
            lightingInfo->advanced_.light0[currentTimeType].color,
            lightingInfo->advanced_.light0[nextTimeType].color,
            lerpFactor);
        float startDir[3];
        float endDir[3];
        float delta[3];
        memcpy(startDir, lightingInfo->advanced_.light0[currentTimeType].direction, 12);
        memcpy(endDir, lightingInfo->advanced_.light0[nextTimeType].direction, 12);
        delta[0] = endDir[0] - startDir[0];
        delta[1] = endDir[1] - startDir[1];
        delta[2] = endDir[2] - startDir[2];
        float lerpX = startDir[0] + (delta[0] * lerpFactor);
        float lerpY = startDir[1] + (delta[1] * lerpFactor);
        float lerpZ = startDir[2] + (delta[2] * lerpFactor);
        outLight0->direction[0] = lerpX;
        outLight0->direction[1] = lerpY;
        outLight0->direction[2] = lerpZ;

        outLight1->color = InterpolateColors(
            lightingInfo->advanced_.light1[currentTimeType].color,
            lightingInfo->advanced_.light1[nextTimeType].color,
            lerpFactor);
        memcpy(startDir, lightingInfo->advanced_.light1[currentTimeType].direction, 12);
        memcpy(endDir, lightingInfo->advanced_.light1[nextTimeType].direction, 12);
        delta[0] = endDir[0] - startDir[0];
        delta[1] = endDir[1] - startDir[1];
        delta[2] = endDir[2] - startDir[2];
        lerpX = startDir[0] + (delta[0] * lerpFactor);
        lerpY = startDir[1] + (delta[1] * lerpFactor);
        lerpZ = startDir[2] + (delta[2] * lerpFactor);
        outLight1->direction[0] = lerpX;
        outLight1->direction[1] = lerpY;
        outLight1->direction[2] = lerpZ;
    }
}

unsigned short LightingManager::InterpolateColors(int col0, int col1, float lerpFactor)
{
    int red0 = col0 & 0x1f;
    char redDiff = (col1 & 0x1f) - (col0 & 0x1f);
    char redLerp = red0 + redDiff * lerpFactor;
    
    int green0 = (col0 & 0x3e0) >> 5;
    char greenDiff = ((col1 & 0x3e0) >> 5) - ((col0 & 0x3e0) >> 5);
    char greenLerp = green0 + greenDiff * lerpFactor;

    int blue0 = (col0 & 0x7c00) >> 10;
    char blueDiff = ((col1 & 0x7c00) >> 10) - ((col0 & 0x7c00) >> 10);
    char blueLerp = blue0 + blueDiff * lerpFactor;

    return redLerp | (greenLerp << 5) | (blueLerp << 10);
}

void LightingManager::ComputeFogInfo(FogInfo *outFog)
{
    Zone3D* zone = pZone_;
    if (zone == NULL)
        return;

    if (lightingIndexOverride_ != 0)
    {
        const FogInfo* source = &zone->lighting_.fogList.entries[lightingIndexOverride_];
        outFog->unk_0 = source->unk_0;
        outFog->type = source->type;
        outFog->depthShift = source->depthShift;
        outFog->offset = source->offset;
        outFog->color = source->color;
        outFog->alpha = source->alpha;
        COPY_ARRAY(outFog->densityTable, source->densityTable);
    }
    else
    {
        (void)GetBattleStruct();
        int currentTimeType = timeOfDayIndex_;
        float endTimes[4] = {
            s_dayThresholds[2], s_dayThresholds[1], s_dayThresholds[0], s_dayThresholds[4]
        };
        float currentTimeValue = dayNightTimer_;
        int nextTimeType = currentTimeType + 1;
        if (nextTimeType > 3)
            nextTimeType = 0;
        float transitionTime = (lightRGBScale_ / 4096.0f) * (currentTimeValue - (endTimes[currentTimeType] - s_dayTransitionLength));
        float lerpFactor;
        if (transitionTime < 0.0f)
            lerpFactor = 0.0f;
        else
            lerpFactor = transitionTime / s_dayTransitionLength;
        
        FogInfo oldFog;
        FogInfo newFog;
        memcpy(&oldFog, &zone->lighting_.fogList.entries[currentTimeType], sizeof(FogInfo));
        memcpy(&newFog, &zone->lighting_.fogList.entries[nextTimeType], sizeof(FogInfo));

        int red0 = oldFog.color & 0x1f;
        char redDiff = (newFog.color & 0x1f) - (oldFog.color & 0x1f);
        char redLerp = red0 + redDiff * lerpFactor;
        
        int green0 = (oldFog.color & 0x3e0) >> 5;
        char greenDiff = ((newFog.color & 0x3e0) >> 5) - ((oldFog.color & 0x3e0) >> 5);
        char greenLerp = green0 + greenDiff * lerpFactor;

        int blue0 = (oldFog.color & 0x7c00) >> 10;
        char blueDiff = ((newFog.color & 0x7c00) >> 10) - ((oldFog.color & 0x7c00) >> 10);
        char blueLerp = blue0 + blueDiff * lerpFactor;

        int alpha0 = oldFog.alpha;
        char alphaDiff = newFog.alpha - oldFog.alpha;
        int alphaLerp = alpha0 + alphaDiff * lerpFactor;

        outFog->color = redLerp | (greenLerp << 5) | (blueLerp << 10);
        outFog->alpha = (char)alphaLerp;
        outFog->type = oldFog.type;
        outFog->depthShift = oldFog.depthShift;

        int offset0 = oldFog.offset;
        int offsetDiff = newFog.offset - oldFog.offset;
        int offsetLerp = offset0 + offsetDiff * lerpFactor;
        outFog->offset = offsetLerp;

        for (int i = 0; i < 32; i++)
        {
            unsigned char table0 = oldFog.densityTable[i];
            char tableDiff = newFog.densityTable[i] - oldFog.densityTable[i];
            outFog->densityTable[i] = table0 + tableDiff * lerpFactor;
        }
    }
}

void LightingManager::Initialize()
{
    VectorizedMemset(this, 0, sizeof(LightingManager));
    lightingIndexOverride_ = 0;
    fogInfo_.unk_0 = 0;
    fogInfo_.type = 0;
    fogInfo_.depthShift = 1;
    fogInfo_.offset = 0x5800;
    fogInfo_.color = 0x7fff;
    fogInfo_.alpha = 15;
    for (int i = 0; i < 0x20; i++)
        fogInfo_.densityTable[i] = 2 * i;
    unknown_84 = 1;
    fogEnabled_ = true;

    RenderConfig::SetLightVector(0, 0, 0, 0);
    RenderConfig::SetLightVector(1, 0, 0, 0);
    RenderConfig::SetLightVector(2, 0, 0, 0);
    RenderConfig::SetLightVector(3, 0, 0, 0);
    RenderConfig::SetLightColor(0, 0);
    RenderConfig::SetLightColor(1, 0);
    RenderConfig::SetLightColor(2, 0);
    RenderConfig::SetLightColor(3, 0);

    lightRGBScale_ = 1 << 12;
    lightRGBScaleInitial_ = 1 << 12;
    lightRGBScaleFinal_ = 1 << 12;
    lightRGBScaleTransitionDuration_ = 0;
    lightRGBScaleTransitionTimer_ = 0;
}

void LightingManager::SetZone(Zone3D *zone) { pZone_ = zone; }

void LightingManager::ModelTransformTintBrightnessContrast(NSBXXInternalModel *model)
{
    BattleStruct* battle = GetBattleStruct();
    (void)func_ov017_0218b5b0();
    
    if (pZone_ == NULL)
        return;
    Zone3D* zone = pZone_;
    LightingInfo* lightingInfo = &pZone_->lighting_;

    unsigned int meshIndex;
    unsigned char* fifoBlob;
    uintptr_t readPos;
    uint32_t* operandPtr;
    int totalNumOperands;
    readPos = (uintptr_t)lightingInfo; // cursed dont look
    
    char* struct0205ec34 = func_0205ec34();

    if (((LightingInfo*)readPos)->maybeMode_ != 1)
        return;

    int index;
    if (zone->currentZoneID_ == 6401 && func_0206dfb0(struct0205ec34, struct0205ec34 + 0x8c, 0x2a))
        index = 4;
    else
    {
        index = lightingIndexOverride_;
        if (index == 0)
        {
            float endTimes[4] = {
                s_dayThresholds[2], s_dayThresholds[1], s_dayThresholds[0], s_dayThresholds[4]
            };
            index = timeOfDayIndex_;
            if (dayNightTimer_ > endTimes[timeOfDayIndex_] - 2.0f * s_dayTransitionLength)
            {
                index++;
                if (index >= 4)
                    index = 0;
                timeOfDayIndex_ = index;
                float beginTimes[4] = {
                    s_dayThresholds[3], s_dayThresholds[2], s_dayThresholds[1], s_dayThresholds[0]
                };
                dayNightTimer_ = beginTimes[timeOfDayIndex_];
                func_02010288(battle, dayNightTimer_);
            }
        }
    }

    NSBXXNameList* meshList = model->GetMeshListSafe();
    
    lightingInfo->basic_.FillMissingEntries();
    func_0207ba28(lightingInfo, index, &redMix_2c_, &greenMix_30_, &blueMix_34_);
    mixBrightness_ = lightingInfo->basic_.unk_70[index];
    mixContrast_ = lightingInfo->basic_.unk_54[index];
    mixContrast_ *= (mixContrast_ * mixContrast_);

    maybePotBarrelDiffuseColor_ = lightingInfo->basic_.potBarrelDiffuseColor[index];
    spriteDiffuseColor_ = lightingInfo->basic_.spriteDiffuseColor[index];
    modelDiffuseColor_ = lightingInfo->basic_.modelDiffuseColor[index];
    edgeColor_ = lightingInfo->basic_.edgeColor[index];
    RenderConfig::SetDiffuseAmbientColors(modelDiffuseColor_, 0, false);
    func_ov017_021901ac(func_ov017_0218b5b0());
    tint_4a_ = modelDiffuseColor_;

    
    for (meshIndex = 0; meshIndex < model->numMeshes_; meshIndex++)
    {
        unsigned char opcode0;
        unsigned char opcode1;
        unsigned char opcode2;
        unsigned char opcode3;

        NSBXXMesh* mesh;
        void* pOffset;
        if (meshList != NULL && (pOffset = meshList->GetEntryv3Safe(meshIndex)))
            mesh = (NSBXXMesh*)meshList->GetEntryFromPtrOffset(pOffset);
        else
            mesh = NULL;
        
        if (!(mesh->unk_4 & 2))
            continue;

        fifoBlob = (unsigned char*)mesh + mesh->gpuCommandsOffset_;
        readPos = 0;
        while (readPos < mesh->gpuCommandsLength_)
        {
            opcode0 = fifoBlob[readPos];
            opcode1 = fifoBlob[readPos + 1];
            opcode2 = fifoBlob[readPos + 2];
            opcode3 = fifoBlob[readPos + 3];
            readPos += 4;

            int oneCommandNumOperands = 0;
            operandPtr = (uint32_t*)(fifoBlob + readPos);
            
            totalNumOperands = 0;

            if (GetFifoCommandData(opcode0, &oneCommandNumOperands))
                operandPtr[totalNumOperands] = ColorTransformTintBrightnessContrast(operandPtr[totalNumOperands]);
            totalNumOperands += oneCommandNumOperands;

            if (GetFifoCommandData(opcode1, &oneCommandNumOperands))
                operandPtr[totalNumOperands] = ColorTransformTintBrightnessContrast(operandPtr[totalNumOperands]);
            totalNumOperands += oneCommandNumOperands;

            if (GetFifoCommandData(opcode2, &oneCommandNumOperands))
                operandPtr[totalNumOperands] = ColorTransformTintBrightnessContrast(operandPtr[totalNumOperands]);
            totalNumOperands += oneCommandNumOperands;

            if (GetFifoCommandData(opcode3, &oneCommandNumOperands))
                operandPtr[totalNumOperands] = ColorTransformTintBrightnessContrast(operandPtr[totalNumOperands]);
            totalNumOperands += oneCommandNumOperands;

            readPos += 4 * totalNumOperands;
        }
    }
}

void LightingManager::ProcessZoneChange(Zone3D *newZone)
{
    pZone_ = newZone;
    Zone3D* zone = pZone_; // why make this copy? but we have to!
    BattleStruct* battle;
    int index;
    LightingInfo* info = &zone->lighting_;    
    battle = GetBattleStruct();
    func_ov017_0218b5b0();
    char* struct0205ec34 = func_0205ec34();

    if (info->maybeMode_ == 2)
    {
        info->advanced_.FillMissingEntries();
        info->fogList.FillMissingEntries();
        RenderConfig::SetDiffuseAmbientColors(modelDiffuseColor_, 0, false);
        func_ov017_021901ac(func_ov017_0218b5b0());
        tint_4a_ = modelDiffuseColor_;
    }
    else if (info->maybeMode_ == 1)
    {
        info->basic_.FillMissingEntries();
        
        // 6401 = id of front of starflight express
        if (zone->currentZoneID_ == 6401 && func_0206dfb0(struct0205ec34, struct0205ec34 + 0x8c, 0x2a))
        {
            index = 4;
        }
        else
        {
            index = lightingIndexOverride_;
            if (index == 0)
            {
                float thresholds[4] = {
                    s_dayThresholds[2], s_dayThresholds[1], s_dayThresholds[0], s_dayThresholds[4]
                };
                index = timeOfDayIndex_;
                if (dayNightTimer_ > thresholds[index] - (2.0f * s_dayTransitionLength))
                {
                    index++;
                    if (index >= 4)
                        index = 0;
                    timeOfDayIndex_ = index;
                    float thresholds2[4] = {
                        s_dayThresholds[3], s_dayThresholds[2], s_dayThresholds[1], s_dayThresholds[0]
                    };
                    dayNightTimer_ = thresholds2[timeOfDayIndex_];
                    func_02010288(battle, dayNightTimer_);
                }
            }
        }

        info->fogList.FillMissingEntries();
        FogInfo* fog = &info->fogList.entries[index];
        maybePotBarrelDiffuseColor_ = info->basic_.potBarrelDiffuseColor[index];
        spriteDiffuseColor_ = info->basic_.spriteDiffuseColor[index];
        modelDiffuseColor_ = info->basic_.modelDiffuseColor[index];
        edgeColor_ = info->basic_.edgeColor[index];
        func_020c54a4(true, fog->type, fog->depthShift, fog->offset);
        GX_FOG_COLOR = fog->color | (fog->alpha << 16);
        func_020c5574(fog->densityTable);
        unsigned short edgeColors[8] = {
            edgeColor_, edgeColor_, edgeColor_, edgeColor_,
            edgeColor_, edgeColor_, edgeColor_, edgeColor_
        };
        func_020c555c(edgeColors);
        RenderConfig::SetDiffuseAmbientColors(modelDiffuseColor_, 0, false);
        func_ov017_021901ac(func_ov017_0218b5b0());
        tint_4a_ = modelDiffuseColor_;
    }
    else // lighting mode is not 1 or 2
    {
        func_020c54a4(0, 0, 1, 0x5800);
        maybePotBarrelDiffuseColor_ = 0x7fff;
        spriteDiffuseColor_ = 0x7fff;
        modelDiffuseColor_ = 0x7fff;
        edgeColor_ = (4 << 10) | (4 << 5) | 6;
        unsigned short edgeColors[8] = {
            edgeColor_, edgeColor_, edgeColor_, edgeColor_,
            edgeColor_, edgeColor_, edgeColor_, edgeColor_
        };
        func_020c555c(edgeColors);
        RenderConfig::SetDiffuseAmbientColors(modelDiffuseColor_, 0, false);
        func_ov017_021901ac(func_ov017_0218b5b0());
        tint_4a_ = modelDiffuseColor_;
    }
    RenderConfig::SetLightVector(0, 0, 0, 0);
    RenderConfig::SetLightVector(1, 0, 0, 0);
    RenderConfig::SetLightVector(2, 0, 0, 0);
    RenderConfig::SetLightVector(3, 0, 0, 0);
    RenderConfig::SetLightColor(0, 0);
    RenderConfig::SetLightColor(1, 0);
    RenderConfig::SetLightColor(2, 0);
    RenderConfig::SetLightColor(3, 0);
}

void LightingManager::RecomputeAdvancedLighting()
{
    Zone3D* zone = pZone_;
    if (zone == NULL)
        return;
    BattleStruct* battle = GetBattleStruct();
    (void)func_ov017_0218b5b0();
    if (zone->lighting_.maybeMode_ == 1 || zone->lighting_.maybeMode_ != 2)
        return;

    Light3DConfig light0;
    Light3DConfig light1;

    GetCurrentAdvancedLightingValues(&ambientColor_, &gradientOuterColor_, &gradientInnerColor_,
        &maybePotBarrelDiffuseColor_, &spriteDiffuseColor_, &modelDiffuseColor_, &edgeColor_,
        &light0, &light1);

    // this is needlessly fiddly to get the rounding right, but ok
    float lightZFix = (light0.direction[2] > 0.0f) ? 
        (4096.0f * light0.direction[2] + 0.5f) :
        (4096.0f * light0.direction[2] - 0.5f);
    float lightYFix = (light0.direction[1] > 0.0f) ? 
        (4096.0f * light0.direction[1] + 0.5f) :
        (4096.0f * light0.direction[1] - 0.5f);
    float lightXFix = (light0.direction[0] > 0.0f) ? 
        (4096.0f * light0.direction[0] + 0.5f) :
        (4096.0f * light0.direction[0] - 0.5f);
    SetVector3fixComponents(&lightVectors_[0], lightXFix, lightYFix, lightZFix);
    Vector3fix_Normalize(&lightVectors_[0], &lightVectors_[0]);
    if (lightVectors_[0].x > 0xff7)
        lightVectors_[0].x = 0xff7;
    if (lightVectors_[0].y > 0xff7)
        lightVectors_[0].y = 0xff7;
    if (lightVectors_[0].z > 0xff7)
        lightVectors_[0].z = 0xff7;
    lightColors_[0] = light0.color;

    lightZFix = (light1.direction[2] > 0.0f) ? 
        (4096.0f * light1.direction[2] + 0.5f) :
        (4096.0f * light1.direction[2] - 0.5f);
    lightYFix = (light1.direction[1] > 0.0f) ? 
        (4096.0f * light1.direction[1] + 0.5f) :
        (4096.0f * light1.direction[1] - 0.5f);
    lightXFix = (light1.direction[0] > 0.0f) ? 
        (4096.0f * light1.direction[0] + 0.5f) :
        (4096.0f * light1.direction[0] - 0.5f);
    SetVector3fixComponents(&lightVectors_[1], lightXFix, lightYFix, lightZFix);
    Vector3fix_Normalize(&lightVectors_[1], &lightVectors_[1]);
    if (lightVectors_[1].x > 0xff7)
        lightVectors_[1].x = 0xff7;
    if (lightVectors_[1].y > 0xff7)
        lightVectors_[1].y = 0xff7;
    if (lightVectors_[1].z > 0xff7)
        lightVectors_[1].z = 0xff7;
    lightColors_[1] = light1.color;

    if (lightRGBScaleTransitionDuration_ != 0)
    {
        int newTimer = lightRGBScaleTransitionTimer_ + func_02010208(battle);
        if (newTimer >= lightRGBScaleTransitionDuration_)
        {
            lightRGBScaleTransitionDuration_ = 0;
            lightRGBScaleTransitionTimer_ = 0;
            lightRGBScale_ = lightRGBScaleFinal_;
            lightRGBScaleInitial_ = lightRGBScaleFinal_;
        }
        else
        {
            lightRGBScaleTransitionTimer_ = newTimer;
            float proportion = (lightRGBScaleTransitionTimer_ / 4096.0f) / (lightRGBScaleTransitionDuration_ / 4096.0f);
            int initial = lightRGBScaleInitial_;
            float initialFloat = initial;
            float totalChange = lightRGBScaleFinal_ - initial;
            lightRGBScale_ = initialFloat + proportion * totalChange;
        }
    }

    if (lightRGBScale_ != 1 << 12)
    {
        unsigned short red = lightColors_[0] & 0x1f;
        unsigned short green = (lightColors_[0] & 0x3e0) >> 5;
        unsigned short blue = (lightColors_[0] & 0x7c00) >> 10;
        float scaleFloat = lightRGBScale_ / 4096.0f;
        red *= scaleFloat;
        green *= scaleFloat;
        blue *= scaleFloat;
        lightColors_[0] = red | (green << 5) | (blue << 10);

        red = lightColors_[1] & 0x1f;
        green = (lightColors_[1] & 0x3e0) >> 5;
        blue = (lightColors_[1] & 0x7c00) >> 10;
        scaleFloat = lightRGBScale_ / 4096.0f;
        red *= scaleFloat;
        green *= scaleFloat;
        blue *= scaleFloat;
        lightColors_[1] = red | (green << 5) | (blue << 10);
    }

    ComputeFogInfo(&fogInfo_);
    if (fogEnabled_)
    {
        func_020c54a4(true, fogInfo_.type, fogInfo_.depthShift, fogInfo_.offset);
        GX_FOG_COLOR = (fogInfo_.color) | (fogInfo_.alpha << 16);
        func_020c5574(fogInfo_.densityTable);
    }
}

void SetVector3fixComponents(Vector3fix* vec, fix32_t x, fix32_t y, fix32_t z)
{
    vec->x = x;
    vec->y = y;
    vec->z = z;
}

void LightingManager::SubmitToRenderConfig()
{
    Zone3D* zone = pZone_;
    if (zone == NULL)
        return;
    (void)func_ov017_0218b5b0();
    RenderConfig::SetDiffuseAmbientColors(modelDiffuseColor_, 0, false);
    if (zone->lighting_.maybeMode_ == 2)
    {
        RenderConfig::SetLightVector(0, lightVectors_[0].x, lightVectors_[0].y, lightVectors_[0].z);
        RenderConfig::SetLightColor(0, lightColors_[0]);
        RenderConfig::SetLightVector(1, lightVectors_[1].x, lightVectors_[1].y, lightVectors_[1].z);
        RenderConfig::SetLightColor(1, lightColors_[1]);
        unsigned short edgeColors[8] = {
            edgeColor_, edgeColor_, edgeColor_, edgeColor_,
            edgeColor_, edgeColor_, edgeColor_, edgeColor_
        };
        func_020c555c(edgeColors);
    }
}

void LightingManager::ApplyAmbientColorToModel(NSBXXInternalModel* model)
{
    NSBXX_Model_SetAmbientReflectionColor(model, ambientColor_);
}

void LightingManager::MaybeComputeHorizonPosition()
{
    void* maybeCameraData = func_020100bc(GetBattleStruct());
    Vector3fix* maybeCameraTarget = (Vector3fix*)((char*)maybeCameraData + 0x12c);
    Vector3fix* maybeCameraEye = (Vector3fix*)((char*)maybeCameraData + 0x120);

    Vector3fix cameraRay;
    Vector3fix_Subtract(maybeCameraTarget, maybeCameraEye, &cameraRay);
    fix32_t rayY = cameraRay.y;
    cameraRay.y = 0;
    Vector3fix_Normalize(&cameraRay, &cameraRay);
    Vector3fixMultiplyScalar(&cameraRay, 2048 << 12, &cameraRay);
    int pixelX, pixelY;
    // convert world coordinates to pixel value
    func_0202ec84(maybeCameraData, &cameraRay, &pixelX, &pixelY);
    if (pixelY < 192 && pixelY > 0)
    {
        gradientCenterNorm_ = fix32_Divide(pixelY << 12, 191 << 12);
        // why do this?
        if (gradientCenterNorm_ < 0)
            gradientCenterNorm_ = -gradientCenterNorm_;
        gradientCenterPixel_ = pixelY;
    }
    else
    {
        // y >= 0: camera is pointing up, horizon is far down
        if (rayY >= 0)
            gradientCenterNorm_ = 1 << 12;
        else
            gradientCenterNorm_ = 0;
    }
}

void LightingManager::DrawBackgroundGradient()
{
    if (pZone_ == NULL)
        return;

    GXFIFO_DIFFUSE_AMBIENT = 0;
    GXFIFO_SPECULAR_EMISSION = 0;
    GXFIFO_TEXIMAGE_PARAMS = 0;
    // alpha = 31, enable front-face rendering and back-face rendering
    GXFIFO_POLYGON_ATTRIBUTES = (0x1f << 16) | (1 << 7) | (1 << 6);
    (void*)GetBattleStruct();
    unsigned short outerColor;
    unsigned short innerColor;
    LightingInfo* lightingInfo = &pZone_->lighting_;
    Struct_ov017_44C8* ov17thing = func_ov017_0218b5b0();
    if (lightingInfo->maybeMode_ == 1)
    {
        int index = timeOfDayIndex_;
        if (lightingIndexOverride_ != 0)
            index = lightingIndexOverride_;
        outerColor = lightingInfo->basic_.backgroundColor[index];
        innerColor = lightingInfo->basic_.backgroundSecondColor[index];
    }
    else
    {
        if (lightingInfo->maybeMode_ != 2)
            return;
        outerColor = gradientOuterColor_;
        innerColor = gradientInnerColor_;
        void* ov17inner = ov17thing->unknown_ptr_3718;
        if (ov17inner != NULL)
        {
            void* ov0thing = func_ov017_021b8468(ov17inner);
            if (ov0thing != NULL && func_ov000_02160fd4(ov0thing, 1 << 9))
            {
                unsigned char* overrideData = (unsigned char*)func_ov017_021b8478(ov17inner);
                int index = overrideData[5];
                outerColor = lightingInfo->advanced_.backgroundColor[index];
                innerColor = lightingInfo->advanced_.backgroundSecondColor[index];
            }
        }
    }
        
    fix16_t yExtra = 4096.0f * lightingInfo->gradientCenterOffset_;
    
    int outerRed, outerGreen, outerBlue;
    int innerRed, innerGreen, innerBlue;

    outerRed = outerColor & 0x1f;
    innerRed = innerColor & 0x1f;
    outerGreen = (outerColor & 0x3e0) >> 5;
    outerBlue = (outerColor & 0x7c00) >> 10;
    innerGreen = (innerColor & 0x3e0) >> 5;
    innerBlue = (innerColor & 0x7c00) >> 10;

    float deltaRed = (outerRed - innerRed) / 0.5f;
    float deltaGreen = (outerGreen - innerGreen) / 0.5f;
    float deltaBlue = (outerBlue - innerBlue) / 0.5f;
    if (lightRGBScale_ != 1 << 12)
    {
        float scale = lightRGBScale_ / 4096.0f;
        outerRed *= scale;
        outerGreen *= scale;
        outerBlue *= scale;
        innerRed *= scale;
        innerGreen *= scale;
        innerBlue *= scale;
        deltaRed *= scale;
        deltaGreen *= scale;
        deltaBlue *= scale;
    }

    unsigned short bottomColor = (outerRed) | (outerGreen << 5) | (outerBlue << 10);
    unsigned short topColor = bottomColor;
    unsigned short middleColor = innerRed | (innerGreen << 5) | (innerBlue << 10);
    
    fix32_t center = gradientCenterNorm_ + yExtra;
    if (center >= 0x800)
    {
        float shorterLength = 1.0f - (center / 4096.0f);
        unsigned char finalInnerRed = innerRed + (int)(deltaRed * shorterLength);
        unsigned char finalInnerGreen = innerGreen + (int)(deltaGreen * shorterLength);
        unsigned char finalInnerBlue = innerBlue + (int)(deltaBlue * shorterLength);
        bottomColor = finalInnerRed | (finalInnerGreen << 5) | (finalInnerBlue << 10);
    }
    else
    {
        float shorterLength = center / 4096.0f;
        unsigned char finalInnerRed = innerRed + (int)(deltaRed * shorterLength);
        unsigned char finalInnerGreen = innerGreen + (int)(deltaGreen * shorterLength);
        unsigned char finalInnerBlue = innerBlue + (int)(deltaBlue * shorterLength);
        topColor = finalInnerRed | (finalInnerGreen << 5) | (finalInnerBlue << 10);
    }
    GXFIFO_MATRIX_PUSH = 0;
    GXFIFO_MATRIX_TRANSLATE = 0;
    GXFIFO_MATRIX_TRANSLATE = 0;
    GXFIFO_MATRIX_TRANSLATE = -1024 << 12;
    GXFIFO_MATRIX_SCALE = 256 << 12;
    GXFIFO_MATRIX_SCALE = 192 << 12;
    GXFIFO_MATRIX_SCALE = 1 << 12;
    GXFIFO_POLYGON_BEGIN = 1;
    GXFIFO_VERTEX_COLOR = bottomColor;

    // bottom left vertex
    GXFIFO_VERTEX_16 = ((1 << 12) << 16) | 0;
    GXFIFO_VERTEX_16 = 1 << 12;
    // bottom right vertex
    GXFIFO_VERTEX_16 = ((1 << 12) << 16) | (1 << 12);
    GXFIFO_VERTEX_16 = 1 << 12;

    GXFIFO_VERTEX_COLOR = middleColor;
    // middle right
    GXFIFO_VERTEX_16 = ((unsigned short)(short)(gradientCenterNorm_ + yExtra)) << 16 | (1 << 12);
    GXFIFO_VERTEX_16 = 1 << 12;
    // middle left
    GXFIFO_VERTEX_16 = ((unsigned short)(short)(gradientCenterNorm_ + yExtra)) << 16;
    GXFIFO_VERTEX_16 = 1 << 12;
    // middle left again
    GXFIFO_VERTEX_16 = ((unsigned short)(short)(gradientCenterNorm_ + yExtra)) << 16;
    GXFIFO_VERTEX_16 = 1 << 12;
    // middle right again
    GXFIFO_VERTEX_16 = ((unsigned short)(short)(gradientCenterNorm_ + yExtra)) << 16 | (1 << 12);
    GXFIFO_VERTEX_16 = 1 << 12;

    GXFIFO_VERTEX_COLOR = topColor;
    // top right
    GXFIFO_VERTEX_16 = (0 << 16) | (1 << 12);
    GXFIFO_VERTEX_16 = 1 << 12;
    // top left
    GXFIFO_VERTEX_16 = (0 << 16) | 0;
    GXFIFO_VERTEX_16 = 1 << 12;

    GXFIFO_POLYGON_END = 0;
    GXFIFO_MATRIX_POP = 1;
}

void LightingManager::BeginFade(fix16_t targetBrightness, unsigned short length)
{
    if (length == 0 || targetBrightness == lightRGBScale_)
    {
        lightRGBScale_ = targetBrightness;
        lightRGBScaleInitial_ = targetBrightness;
        lightRGBScaleFinal_ = targetBrightness;
        lightRGBScaleTransitionDuration_ = 0;
        lightRGBScaleTransitionTimer_ = 0;
    }
    else
    {
        lightRGBScaleInitial_ = lightRGBScale_;
        lightRGBScaleFinal_ = targetBrightness;
        lightRGBScaleTransitionDuration_ = length;
        lightRGBScaleTransitionTimer_ = 0;
    }
}

TimeOfDay ConvertToTimeOfDay(float time)
{
    if (time < 0.0f || s_dayThresholds[4] < time)
        return TimeOfDay_Invalid;
    
    if (time >= s_dayThresholds[0])
        return TimeOfDay_Evening;
    if (time >= s_dayThresholds[1])
        return TimeOfDay_Day;
    if (time >= s_dayThresholds[2])
        return TimeOfDay_Morning;
    return TimeOfDay_Night;
}

LightingManager::LightingManager()
{
    Initialize();
}