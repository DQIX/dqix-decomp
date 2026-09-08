#pragma once

#include "NSBXX/NSBXX.h"
#include "NSBXX/RenderConfig.h"
#include "Vector.h"

class Zone3D;

struct Vector3float
{
    float x;
    float y;
    float z;
};

enum TimeOfDay
{
    TimeOfDay_Invalid = -1,
    TimeOfDay_Night = 0,
    TimeOfDay_Morning = 1,
    TimeOfDay_Day = 2,
    TimeOfDay_Evening = 3
};

// sizeof == 0x9c.
// An instance of this exists at 0x02107930 (usa).
// Manages lighting-related stuff for a zone, e.g. diffuse colors for models
// and sprites, fog color/alpha/visibility and configuration of (two of) the
// DS's 3D lights. Unlike LightingInfo which holds all values associated to a
// single zone, this holds only the current values for the current zone.
// 
// There are two 'modes' of lighting: the 'basic' mode (enum value 1), used for
// more populated zones e.g. towns, which does not use the 3D lights, and the
// 'advanced' mode (enum value 2), used for fields and battlefields (note not
// caves etc, only open-world fields). Each zone has specific values for colors etc.
// at each time of day, all stored in arrays (with [0] = night, [1] = morning etc
// as the enum above). For some reason all arrays are of length 7 though. Some
// zones specify entries for indices 4 and up, but they're usually default values.
// One notable exception is the front carriage of the Starflight Express (id 6401 = 0x1901)
// which actually uses index 4 for rendering
// 
class LightingManager
{
public:
    struct FogInfo
    {
        // seems to be a bool based on script initialization, 
        // maybe 'is fog enabled'
        int unk_0;
        int type; // 0 = color and alpha, 1 = alpha only
        int depthShift;
        int offset;
        unsigned short color;
        int alpha;
        unsigned char densityTable[0x20];
    };

    // sizeof == 0xe8
    // aka 'mode 1', used in towns/buildings/caves
    struct BasicLighting
    {
        Vector3float unk_0[7];
        float unk_54[7];
        float unk_70[7];
        // clear color when out of bounds
        unsigned short backgroundColor[7];
        unsigned short backgroundSecondColor[7];
        // not sure if this is used/supposed to be used more broadly, but changing
        // it at runtime affected these
        unsigned short potBarrelDiffuseColor[7];
        unsigned short spriteDiffuseColor[7];
        unsigned short modelDiffuseColor[7];
        unsigned short edgeColor[7];
        bool unk_e0[7];
        char padding_e7;

        // see FogList member function of the same name
        void FillMissingEntries();
    };

    // describes the settings for one of the DS's four 3D lights
    struct Light3DConfig
    {
        unsigned short color;
        float direction[3]; // not sure of type
        bool maybeEnabled;
    };

    struct AdvancedLighting
    {
        Light3DConfig light0[7];
        Light3DConfig light1[7];
        unsigned short maybeAmbientColor[7];
        // clear color when out of bounds or for e.g. sky. If the background
        // is a gradient, this is the color at the top and bottom
        unsigned short backgroundColor[7];
        // if the background is a vertical gradient, e.g. for the sky in
        // battlefields, this is the color in the middle (i.e. at the horizon)
        unsigned short backgroundSecondColor[7];
        // given basic lighting, this is probably the color of pots/barrels
        // but I don't think there are any in zones that use this lighting
        unsigned short unk_142[7];
        unsigned short spriteDiffuseColor[7];
        // applies to the player. in battlefields also applies to enemies
        unsigned short modelDiffuseColor[7]; 
        unsigned short edgeColor[7];
        char padding_17a[2];

        // see FogList member function of the same name
        void FillMissingEntries();
    };

    Zone3D* pZone_;
    Vector3fix lightVectors_[2];
    unsigned short lightColors_[2];
    // assumed to be <= 1, scales RGB values, behaves weirdly if > 1 due to
    // R values over 31 spilling into G etc
    fix16_t lightRGBScale_;
    fix16_t lightRGBScaleInitial_;
    fix16_t lightRGBScaleFinal_;
    unsigned short lightRGBScaleTransitionDuration_;
    unsigned short lightRGBScaleTransitionTimer_;

    char unk_2a[2];
    
    float redMix_2c_;
    float greenMix_30_;
    float blueMix_34_;
    float mixBrightness_;
    // contrast is additive rather than multiplicative: if the channel value
    // is above 0.5, it is added, if below 0.5 it is subtracted. Values are
    // capped to prevent crossing 0.5 in the case of negative contrast
    float mixContrast_;

    unsigned short ambientColor_;
    unsigned short maybePotBarrelDiffuseColor_;
    unsigned short spriteDiffuseColor_;
    unsigned short modelDiffuseColor_;
    unsigned short edgeColor_;
    unsigned short tint_4a_;

    FogInfo fogInfo_;
    char unknown_84;
    bool fogEnabled_; // probably bool "has fog"

    fix16_t gradientCenterNorm_;
    unsigned int gradientCenterPixel_;
    unsigned short gradientOuterColor_;
    unsigned short gradientInnerColor_;
    // if nonzero, this will be used to index into all the LightingInfo arrays
    // instead of the time of day index. I've never seen it nonzero in normal
    // gameplay but I haven't looked super hard
    int lightingIndexOverride_;
    float dayNightTimer_;
    int timeOfDayIndex_;

    struct FogList
    {
        FogInfo entries[7];

        // Fill invalid entries by copying from the most recent
        // valid entry, i.e. [3] filled from [2] if possible. Only fills
        // entries 0 to 3 and cycles around (so e.g. [0] can be filled from
        // [3])
        void FillMissingEntries();
    };

public:
    static LightingManager* GetInstance();

    // We use this to (destructively) apply color mixing to a mesh by going
    // through and modifying the fifo data blob. Returns true if the opcode
    // has a color operand (if so, it'll be the only operand), and returns
    // by pointer the number of operands so you can skip to the next one
    bool GetFifoCommandData(int opcode, int* outNumArgs);
    unsigned short ColorTransformTintBrightnessContrast(unsigned int inColor);
    void GetCurrentAdvancedLightingValues(unsigned short* outMaybeAmbient,
        unsigned short* outBGColor, unsigned short* outBGSecondColor, unsigned short* outArg4,
        unsigned short* outSpriteDiffuse, unsigned short* outModelDiffuse,
        unsigned short* outEdgeColor, Light3DConfig* outLight0, Light3DConfig* outLight1);

    unsigned short InterpolateColors(int col0, int col1, float lerpFactor);
    void ComputeFogInfo(FogInfo* outFog);

    void Initialize();
    void SetZone(Zone3D* zone);

    void ModelTransformTintBrightnessContrast(NSBXXInternalModel* model);

    void ProcessZoneChange(Zone3D* newZone);

    void RecomputeAdvancedLighting();
    // In basic mode this only sets the model diffuse color.
    // In advanced mode it additionally sets the color and direction of the two
    // 3D lights as well as the edge color.
    void SubmitToRenderConfig();
    void ApplyAmbientColorToModel(NSBXXInternalModel* model);

    void MaybeComputeHorizonPosition();
    void DrawBackgroundGradient();

    void BeginFade(fix16_t targetBrightness, unsigned short length);

    LightingManager();
};

void GetDayThresholds(float* outNightToMorning, float* outMorningToDay,
    float* outDayToEvening, float* outEveningToNight, float* outTransitionTime);
TimeOfDay ConvertToTimeOfDay(float time);