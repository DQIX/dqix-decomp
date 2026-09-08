#include "World/ZoneFeatures.h"
#include "World/Zone3D.h"
#include "Resource/Script.h"
#include "Combat/Main/BattleList.h"
#include "System/Memory.h"

#if defined(jpn)
#define data_020ef388 data_020ef2c4
#define data_020fdc20 data_020fd98c

#define func_02011584 func_020112f4
#define func_0209998c func_0209b6c0
#endif

extern Script::OpcodeLookupEntry data_020ef388[];

struct Struct_020fdc20
{
    ZoneFeatures::Opcode6aEntry* currentEntry;
    SafeAllocator* allocator;
    ZoneFeatures* warp;
} extern data_020fdc20;

extern "C"
{
    void* func_02011584(BattleStruct*);
    // probably get zone data by name
    unsigned short* func_0209998c(void*, const char*);
}

int WarpScript_Opcode_64(Script::Parameter* params, int numParams)
{
    data_020fdc20.warp->AllocateOpcode64Entries(params[0].ToInt(), data_020fdc20.allocator);
    return 1;
}

int WarpScript_Opcode_65(Script::Parameter* params, int numParams)
{
    const char* string2;
    const char* string3;
    int arg0 = params[0].ToInt();
    Vector3fix argVector;
    Script::Parameter* post = params[1].ToVec3fix(&argVector);
    params = post + 1;
    const char* string1 = post->ToString();
    string2 = NULL;
    string3 = NULL;

    if (numParams >= 6)
    {
        string2 = params->ToString();
        params++;
    }
    if (numParams >= 7)
    {
        string3 = params->ToString();
        params++;
    }

    if (string1 == NULL)
        return 0;

    ZoneFeatures::Opcode64Entry entry;
    entry.unk_0 = arg0;
    entry.vector_4.vec = argVector;
    strcpy(entry.string_10, string1);
    if (string2 == NULL)
        entry.string_20[0] = '\0';
    else
        strcpy(entry.string_20, string2);
    if (string3 == NULL)
        entry.string_30[0] = '\0';
    else
        strcpy(entry.string_30, string3);

    data_020fdc20.warp->CreateOpcode64Entry(entry);
    return 1;
}

int WarpScript_Opcode_7d(Script::Parameter* params, int numParams)
{
    data_020fdc20.warp->AllocateGrottoTileFeaturePlacementEntries(params[0].ToInt(), data_020fdc20.allocator);
    return 1;
}

int WarpScript_Opcode_7e(Script::Parameter* params, int numParams)
{
    TileFeaturePlacementData placement;
    placement.tileID = (params++)->ToInt();
    strcpy(placement.tilename, (params++)->ToString());

    for (int i = 0; i < 9; i++)
        placement.directionBitmasks[i] = (params++)->ToInt();

    data_020fdc20.warp->CreateGrottoTileFeaturePlacementEntry(placement);
    return 1;
}

int WarpScript_Opcode_66(Script::Parameter* params, int numParams)
{
    data_020fdc20.warp->AllocateOpcode66Entries(params[0].ToInt(), data_020fdc20.allocator);
    return 1;
}

int WarpScript_Opcode_67(Script::Parameter* params, int numParams)
{
    float arg0 = params[0].ToFloat();
    float arg1 = params[1].ToFloat();
    float arg2 = params[2].ToFloat();
    float arg3 = params[3].ToFloat();
    float arg4 = params[4].ToFloat();
    float arg5 = params[5].ToFloat();
    float arg8;
    int arg6 = params[6].ToInt();
    int arg7 = params[7].ToInt();
    arg8 = 0;
    if (numParams >= 9)
        arg8 = params[8].ToFloat();

    ZoneFeatures::Opcode66Entry entry;
    entry.unk_0[0] = 4096.0f * (arg0 + (arg3 / 2.0f));
    entry.unk_0[1] = 4096.0f * (arg1 + (arg4 / 2.0f));
    entry.unk_0[2] = 4096.0f * (arg2 + (arg5 / 2.0f));

    entry.unk_0[3] = 4096.0f * (arg0 - (arg3 / 2.0f));
    entry.unk_0[4] = 4096.0f * (arg1 - (arg4 / 2.0f));
    entry.unk_0[5] = 4096.0f * (arg2 - (arg5 / 2.0f));

    entry.unk_18[0] = arg6;
    entry.unk_18[1] = arg7;
    entry.unk_20 = 4096.0f * arg8;
    
    data_020fdc20.warp->CreateOpcode66Entry(entry);

    return 1;
}

int WarpScript_Opcode_68(Script::Parameter* params, int numParams)
{
    data_020fdc20.warp->AllocateOpcode68Entries(params[0].ToInt(), data_020fdc20.allocator);
    return 1;
}

bool ProcessExtraOpcode69Params(Script::Parameter* param, int numParams, ZoneFeatures::Opcode68Entry& entry)
{
    Script::Parameter* paramStart = param;
    void* worldData = func_02011584(GetBattleStruct());
    
    entry.unk_0 = (param++)->ToInt();
    if (paramStart[1].type == 0)
    {
        unsigned short* probablyZone = func_0209998c(worldData, (param++)->ToString());
        if (probablyZone == NULL)
            return false;
        entry.unk_1c = *probablyZone;
    }
    else
    {
        entry.unk_1c = (param++)->ToInt();
    }
    entry.unk_64[0] = (param++)->ToInt();
    entry.unk_64[1] = (param++)->ToInt();

    Vector3fix tempVector;
    param = param->ToVec3fix(&tempVector);
    entry.unk_20[0] = tempVector;
    entry.unk_50 = 4096.0f * (param++)->ToFloat();

    if (numParams - (param - paramStart) > 0)
    {
        entry.unk_6c = (param++)->ToInt();
    }
    if (numParams - (param - paramStart) > 0)
    {
        param = param->ToVec3fix(&tempVector);
        entry.unk_20[1] = tempVector;
        param = param->ToVec3fix(&tempVector);
        entry.unk_20[2] = tempVector;
        param = param->ToVec3fix(&tempVector);
        entry.unk_20[3] = tempVector;
    }
    return true;
}

int WarpScript_Opcode_69(Script::Parameter* params, int numParams)
{
    ZoneFeatures::Opcode68Entry entry;
    entry.Reset();
    Script::Parameter* p = params;
    fix32_t centreX = (int)(4096.0f * (p++)->ToFloat());
    fix32_t centreY = (int)(4096.0f * (p++)->ToFloat());
    fix32_t centreZ = (int)(4096.0f * (p++)->ToFloat());
    fix32_t lengthX = (int)(4096.0f * (p++)->ToFloat()) / 2;
    fix32_t lengthY = (int)(4096.0f * (p++)->ToFloat()) / 2;
    fix32_t lengthZ = (int)(4096.0f * (p++)->ToFloat()) / 2;

    fix32_t xMax = centreX + lengthX;
    fix32_t xMin = centreX - lengthX;
    fix32_t yMax = centreY + lengthY;
    fix32_t yMin = centreY - lengthY;
    fix32_t zMax = centreZ + lengthZ;
    fix32_t zMin = centreZ - lengthZ;

    entry.unk_58[1] = centreY;
    entry.unk_4[0] = xMax;
    entry.unk_58[0] = centreX;
    entry.unk_58[2] = centreZ;   

    entry.unk_4[1] = yMax;
    entry.unk_4[2] = zMax;
    entry.unk_4[3] = xMin;
    entry.unk_4[4] = yMin;
    entry.unk_4[5] = zMin;

    if (!ProcessExtraOpcode69Params(p, numParams - (p - params), entry))
        return 0;
    data_020fdc20.warp->CreateOpcode68Entry(entry);
    return 1;
}

void ZoneFeatures::Opcode68Entry::Reset()
{
    unk_0 = 0;
    unk_1c = -1;
    unk_50 = 0;
    unk_52 = 0;
    unk_54 = 0;
    unk_64[0] = 0;
    unk_64[1] = 0;
    unk_58[0] = 0;
    unk_58[1] = 0;
    unk_58[2] = 0;
    unk_20[0].x = 0;
    unk_20[0].y = 0;
    unk_20[0].z = 0;
    unk_20[1].x = 0;
    unk_20[1].y = 0;
    unk_20[1].z = 0;
    unk_20[2].x = 0;
    unk_20[2].y = 0;
    unk_20[2].z = 0;
    unk_20[3].x = 0;
    unk_20[3].y = 0;
    unk_20[3].z = 0;
    unk_6c = -1;
}

int WarpScript_Opcode_72(Script::Parameter* params, int numParams)
{
    Script::Parameter* paramsStart = params;
    ZoneFeatures::Opcode68Entry entry;
    entry.Reset();
    fix32_t centreX = (int)(4096.0f * (params++)->ToFloat());
    fix32_t centreY = (int)(4096.0f * (params++)->ToFloat());
    fix32_t centreZ = (int)(4096.0f * (params++)->ToFloat());
    fix32_t lengthX = (int)(4096.0f * (params++)->ToFloat()) / 2;
    fix32_t lengthY = (int)(4096.0f * (params++)->ToFloat()) / 2;
    fix32_t lengthZ = (int)(4096.0f * (params++)->ToFloat()) / 2;

    entry.unk_4[0] = centreX + lengthX;
    entry.unk_4[1] = centreY + lengthY;
    entry.unk_4[2] = centreZ + lengthZ;
    entry.unk_4[3] = centreX - lengthX;
    entry.unk_4[4] = centreY - lengthY;
    entry.unk_4[5] = centreZ - lengthZ;

    entry.unk_58[0] = centreX;
    entry.unk_58[1] = centreY;
    entry.unk_58[2] = centreZ;

    entry.unk_52 = 4096.0f * (params++)->ToFloat();

    fix32_t xsqu = FIX32_MULTIPLY(lengthX, lengthX);
    fix32_t zsqu = FIX32_MULTIPLY(lengthZ, lengthZ);
    entry.unk_54 = zsqu;
    entry.unk_54 = xsqu + zsqu;

    if (!ProcessExtraOpcode69Params(params, numParams - (params - paramsStart), entry))
        return 0;
    data_020fdc20.warp->CreateOpcode68Entry(entry);
    return 1;
}

int WarpScript_Opcode_6a(Script::Parameter* params, int numParams)
{
    data_020fdc20.warp->AllocateOpcode6aEntries(params[0].ToInt(), data_020fdc20.allocator);
    return 1;
}

int WarpScript_Opcode_6b(Script::Parameter* params, int numParams)
{
    Script::Parameter* paramsStart = params;
    ZoneFeatures::Opcode6aEntry entry;
    entry.Reset();
    int hash = (params++)->ToInt();
    entry.maybeType = hash;
    bool didSubtraction = false;
    if (entry.maybeType >= 10)
    {
        entry.maybeType -= 10;
        didSubtraction = true;
    }

    Vector3fix tempVector;
    params = params->ToVec3fix(&tempVector);
    entry.unk_8.vec = tempVector;
    params = params->ToVec3fix(&tempVector);
    entry.unk_14.vec = tempVector;

    if (didSubtraction)
    {
        fix32_t angle = 4096.0f * (params++)->ToFloat();
        entry.unk_22 = fix32ReduceAngle0To2Pi(angle);
    }
    entry.unk_20 = 4096.0f * (params++)->ToFloat();
    
    fix32_t halfx = entry.unk_14.vec.x / 2;
    fix32_t halfz = entry.unk_14.vec.z / 2;
    fix32_t zsqu = FIX32_MULTIPLY(halfz, halfz);
    fix32_t xsqu = FIX32_MULTIPLY(halfx, halfx);
    entry.unk_24 = zsqu;
    entry.unk_24 = xsqu + zsqu;    

    if (entry.maybeType == 2)
    {
        entry.unk_2c.type2.unk_0 = params[0].ToInt();
        entry.unk_2c.type2.unk_1 = params[1].ToInt();
        entry.unk_2c.type2.unk_2_high = params[2].ToInt();
        params += 3;
    }
    else if (entry.maybeType == 3)
    {
        entry.unk_2c.type3.unk_0 = (params++)->ToInt();
    }
    else if (entry.maybeType == 4)
    {
        int arg = (params++)->ToInt();
        entry.unk_2c.type4.unk_1 = -1;
        entry.unk_2c.type4.unk_0 = arg;
        entry.unk_2c.type4.unk_0_high = 0;
        switch (entry.unk_2c.type4.unk_0)
        {
        case 0:
            entry.unk_2c.type4.vector_4.vec3.vec.y = 4096.0f * (params++)->ToFloat();
            entry.unk_2c.type4.vector_4.vec3.vec.z = 4096.0f * (params++)->ToFloat();
            break;
        case 1:
            params = params->ToVec3fix(&tempVector);
            entry.unk_2c.type4.vector_4.vec3.vec = tempVector;
            break;
        case 2:
        case 3:
            params = params->ToVec3fix(&tempVector);
            entry.unk_2c.type4.vector_4.vec3.vec = tempVector;
            break;
        case 4:
        case 5:
        {
            Vector3fix vecB;
            Vector3fix vecA;
            params = params->ToVec3fix(&vecB);
            params = params->ToVec3fix(&vecA);
            MaybeVector4fix difference;
            Vector3fix_Subtract(&vecA, &vecB, &difference.vec3.vec);
            fix32_t distance = Vector3fix_Length(&difference.vec3.vec);
            Vector3fix_Normalize(&difference.vec3.vec, &difference.vec3.vec);
            difference.w = Vector3fix_InnerProduct(&difference.vec3.vec, &vecB);
            Vector3fix vecC  = { 0, 0, 0 };
            if (entry.unk_2c.type4.unk_0 == 4)
            {
                vecC.x = 4096.0f * (params++)->ToFloat();
            }
            vecC.y = 4096.0f * params[0].ToFloat();
            vecC.z = 4096.0f * params[1].ToFloat();
            Vector3fix vecD;
            params = params[2].ToVec3fix(&vecD);
            entry.unk_2c.type4.vector_4.CopyFrom(difference);
            entry.unk_2c.type4.distance_14 = distance;
            entry.unk_2c.type4.vector_18 = vecC;
            entry.unk_2c.type4.vector_24 = vecD;
            break;
        }
        }
        entry.unk_2c.type4.vector_30.x = 0;
        entry.unk_2c.type4.vector_30.y = 0;
        entry.unk_2c.type4.vector_30.z = 0;
        if (numParams > 12)
        {
            params = params->ToVec3fix(&tempVector);
            entry.unk_2c.type4.vector_30 = tempVector;
        }
    }
    else if (entry.maybeType == 5)
    {
        entry.unk_2c.type5.unk_0 = params[0].ToInt();
        entry.unk_2c.type5.unk_2 = params[1].ToInt();
        entry.unk_2c.type5.unk_4 = 0;
    }
    Struct_020fdc20* instance = &data_020fdc20;
    instance++; // do some nonsense to disable propagation optimizations
    entry.pNext = NULL;
    (instance - 1)->warp->CreateOpcode6aEntry(entry);
    return 1;
}

void ZoneFeatures::Opcode6aEntry::Reset()
{
    VectorizedMemset(this, 0, sizeof(Opcode6aEntry));
    unk_0 = 0;
    maybeType = 0;
    unk_20 = 0;
    unk_22 = 0;
    unk_24 = 0;
    unk_8.vec.x = 0;
    unk_8.vec.y = 0;
    unk_8.vec.z = 0;
    unk_14.vec.x = 0;
    unk_14.vec.y = 0;
    unk_14.vec.z = 0;
    unk_28 = 0;
    pNext = NULL;
}

MaybeVector4fix &MaybeVector4fix::CopyFrom(const MaybeVector4fix &other)
{
    vec3 = other.vec3;
    fix32_t sourceW = other.w; // prevents inlining
    w = sourceW;
    return *this;
}

int WarpScript_Opcode_6e(Script::Parameter* params, int numParams)
{
    Vector3fix tempVector;
    params = params->ToVec3fix(&tempVector);
    fix32_t angle = fix32ReduceAngle0To2Pi(4096.0f * (params++)->ToFloat());

    ZoneFeatures* warp = data_020fdc20.warp;
    warp->vector_70_ = tempVector;
    warp->angle_7c_ = angle;
    return 1;
}

int WarpScript_Opcode_70(Script::Parameter* params, int numParams)
{
    int red = (params++)->ToInt();
    int green = (params++)->ToInt();
    int blue = (params++)->ToInt();
    data_020fdc20.warp->color_7e_ = red | (green << 5) | (blue << 10);
    return 1;
}

int WarpScript_Opcode_73(Script::Parameter* params, int numParams)
{
    ZoneFeatures::Opcode6aEntry entry;
    entry.Reset();
    entry.maybeType = (params++)->ToInt();
    Vector3fix tempVector;
    params = params->ToVec3fix(&tempVector);
    entry.unk_8.vec = tempVector;
    params = params->ToVec3fix(&tempVector);
    entry.unk_14.vec = tempVector;
    
    entry.unk_22 = fix32ReduceAngle0To2Pi(4096.0f * (params++)->ToFloat());
    entry.unk_20 = 4096.0f * (params++)->ToFloat();

    fix32_t halfx = entry.unk_14.vec.x / 2;
    fix32_t halfz = entry.unk_14.vec.z / 2;
    fix32_t xsqu = FIX32_MULTIPLY(halfx, halfx);
    fix32_t zsqu = FIX32_MULTIPLY(halfz, halfz);
    entry.unk_24 = zsqu;
    entry.unk_24 = xsqu + zsqu;

    data_020fdc20.currentEntry = data_020fdc20.warp->CreateOpcode6aEntry(entry);
    return 1;
}

int WarpScript_Opcode_74(Script::Parameter* params, int numParams)
{
    if (data_020fdc20.currentEntry == NULL)
        return 0;
    void* worldData = func_02011584(GetBattleStruct());
    switch (data_020fdc20.currentEntry->maybeType)
    {
    case 0:
        data_020fdc20.currentEntry->unk_2c.type0.unk_0 = 0;
        if (numParams != 0)
            data_020fdc20.currentEntry->unk_0 = params->ToInt();
        break;
    case 6:
        data_020fdc20.currentEntry->maybeType = 0;
        data_020fdc20.currentEntry->unk_2c.type0.unk_0 = 1;
        if (numParams != 0)
            data_020fdc20.currentEntry->unk_0 = params->ToInt();
        break;
    case 1:
        if (numParams != 0)
            data_020fdc20.currentEntry->unk_0 = params->ToInt();
        break;
    case 2:
        if (numParams == 3)
        {
            data_020fdc20.currentEntry->unk_2c.type2.unk_0 = (params++)->ToInt();
            data_020fdc20.currentEntry->unk_2c.type2.unk_1 = (params++)->ToInt();
            data_020fdc20.currentEntry->unk_2c.type2.unk_2_high = (params++)->ToInt();
            data_020fdc20.currentEntry->unk_2c.type2.unk_2_low = 0;
        }
        else
        {
            int numArgsConsumed = 0;
            data_020fdc20.currentEntry->unk_2c.type2.unk_0 = (params++)->ToInt();
            numArgsConsumed++;
            data_020fdc20.currentEntry->unk_2c.type2.unk_1 = (params++)->ToInt();
            numArgsConsumed++;
            data_020fdc20.currentEntry->unk_2c.type2.unk_2_high = (params++)->ToInt();
            numArgsConsumed++;
            data_020fdc20.currentEntry->unk_2c.type2.unk_2_low = (params++)->ToInt();
            numArgsConsumed++;
            
            if (data_020fdc20.currentEntry->unk_2c.type2.unk_2_high & 8)
            {
                if (params->type == 0)
                {
                    const char* zoneName = (params++)->ToString();
                    numArgsConsumed++;
                    unsigned short* maybeZoneData = func_0209998c(worldData, zoneName);
                    
                    if (maybeZoneData == NULL)
                        return 0;
                    data_020fdc20.currentEntry->unk_2c.type2.maybeZone = *maybeZoneData;
                }
                else
                {
                    data_020fdc20.currentEntry->unk_2c.type2.maybeZone = (params++)->ToInt();
                    numArgsConsumed++;
                }
                data_020fdc20.currentEntry->unk_2c.type2.unk_6 = (params++)->ToInt();
                numArgsConsumed++;
                data_020fdc20.currentEntry->unk_2c.type2.unk_8 = (params++)->ToInt();
                numArgsConsumed++;
                Vector3fix tempVector;
                params = params->ToVec3fix(&tempVector);
                numArgsConsumed += 3;
                data_020fdc20.currentEntry->unk_2c.type2.vectors_c[0] = tempVector;
                data_020fdc20.currentEntry->unk_2c.type2.unk_3c = 4096.0f * (params++)->ToFloat();
                numArgsConsumed++;
                data_020fdc20.currentEntry->unk_2c.type2.unk_3e = 4096.0f * (params++)->ToFloat();
                numArgsConsumed++;
                
                for (int i = 1; i < 4; i++)
                {
                    params = params->ToVec3fix(&tempVector);
                    numArgsConsumed += 3;
                    data_020fdc20.currentEntry->unk_2c.type2.vectors_c[i] = tempVector;
                }
            }
            if (numArgsConsumed < numParams)
            {
                (void)((params++)->ToFloat());
                numArgsConsumed++;
            }
            if (numArgsConsumed < numParams)
            {
                (void)((params++)->ToInt());
                numArgsConsumed++;
            }
            if (data_020fdc20.currentEntry->unk_2c.type2.unk_2_high & 0x20)
            {
                data_020fdc20.currentEntry->unk_2c.type2.unk_40 = 4096.0f * (params++)->ToFloat();
            }
        }
        break;
    case 3:
        data_020fdc20.currentEntry->unk_2c.type3.unk_0 = (params++)->ToInt();
        break;
    case 4:
    {
        data_020fdc20.currentEntry->unk_2c.type4.unk_0 = (params++)->ToInt();
        data_020fdc20.currentEntry->unk_2c.type4.unk_1 = (params++)->ToInt();
        data_020fdc20.currentEntry->unk_2c.type4.unk_0_high = (params++)->ToInt();
        Vector3fix vecA;
        Vector3fix vecB;
        params = params->ToVec3fix(&vecA);
        params = params->ToVec3fix(&vecB);
        MaybeVector4fix vector4;
        Vector3fix_Subtract(&vecB, &vecA, &vector4.vec3.vec);
        fix32_t distance = Vector3fix_Length(&vector4.vec3.vec);
        Vector3fix_Normalize(&vector4.vec3.vec, &vector4.vec3.vec);
        vector4.w = Vector3fix_InnerProduct(&vector4.vec3.vec, &vecA);
        Vector3fix vecC = { 0, 0, 0 };
        if (data_020fdc20.currentEntry->unk_2c.type4.unk_0 == 4)
        {
            params = params->ToVec3fix(&vecC);
        }
        else
        {
            vecC.y = 4096.0f * (params++)->ToFloat();
            vecC.z = 4096.0f * (params++)->ToFloat();
        }
        Vector3fix vecD;
        params = params->ToVec3fix(&vecD);
        data_020fdc20.currentEntry->unk_2c.type4.vector_4.CopyFrom(vector4);
        data_020fdc20.currentEntry->unk_2c.type4.distance_14 = distance;
        data_020fdc20.currentEntry->unk_2c.type4.vector_18 = vecC;
        data_020fdc20.currentEntry->unk_2c.type4.vector_24 = vecD;
        break;
    }
    case 5:
        data_020fdc20.currentEntry->unk_2c.type5.unk_0 = (params++)->ToInt();
        data_020fdc20.currentEntry->unk_2c.type5.unk_2 = (params++)->ToInt();
        if (numParams > 2)
            data_020fdc20.currentEntry->unk_0 = (params++)->ToInt();
        break;
    case 8:
        if (numParams != 0)
            data_020fdc20.currentEntry->unk_0 = (params++)->ToInt();
        break;
    case 9:
        data_020fdc20.currentEntry->unk_2c.type9.unk_0 = (params++)->ToInt();
        data_020fdc20.currentEntry->unk_2c.type9.unk_2 = (params++)->ToInt();
        data_020fdc20.currentEntry->unk_2c.type9.unk_4 = (params++)->ToInt();
        data_020fdc20.currentEntry->unk_2c.type9.unk_5 = (params++)->ToInt();
        if (data_020fdc20.currentEntry->unk_2c.type9.unk_5 & 2)
        {
            if (params->type == 0)
            {
                const char* zoneName = (params++)->ToString();
                unsigned short* maybeZoneData = func_0209998c(worldData, zoneName);
                
                if (maybeZoneData == NULL)
                    return 0;
                data_020fdc20.currentEntry->unk_2c.type9.maybeZone = *maybeZoneData;
            }
            else
            {
                data_020fdc20.currentEntry->unk_2c.type9.maybeZone = (params++)->ToInt();
            }
            data_020fdc20.currentEntry->unk_2c.type9.unk_8 = (params++)->ToInt();
            data_020fdc20.currentEntry->unk_2c.type9.unk_a = (params++)->ToInt();
            params = params->ToVec3fix(&data_020fdc20.currentEntry->unk_2c.type9.vector_c);
            data_020fdc20.currentEntry->unk_2c.type9.unk_3c = 4096.0f * (params++)->ToFloat();
            data_020fdc20.currentEntry->unk_2c.type9.unk_3e = 4096.0f * (params++)->ToFloat();
            params = params->ToVec3fix(&data_020fdc20.currentEntry->unk_2c.type9.vector_18);
            params = params->ToVec3fix(&data_020fdc20.currentEntry->unk_2c.type9.vector_24);
            params = params->ToVec3fix(&data_020fdc20.currentEntry->unk_2c.type9.vector_30);
            data_020fdc20.currentEntry->unk_2c.type9.unk_40 = 4096.0f * (params++)->ToFloat();
            data_020fdc20.currentEntry->unk_2c.type9.unk_42 = (params++)->ToInt();
        }
        break;
    case 10:
        data_020fdc20.currentEntry->unk_2c.type10.unk_0 = (params++)->ToInt();
        params = params->ToVec3fix(&data_020fdc20.currentEntry->unk_2c.type10.vector_4);
        data_020fdc20.currentEntry->unk_2c.type10.unk_2 = 4096.0f * (params++)->ToFloat();
        data_020fdc20.currentEntry->unk_2c.type10.unk_10 = 4096.0f * (params++)->ToFloat();
        if (numParams > 6)
        {
            params = params->ToVec3fix(&data_020fdc20.currentEntry->unk_2c.type10.vector_14);
            data_020fdc20.currentEntry->unk_2c.type10.unk_20 = 4096.0f * (params++)->ToFloat();
        }
        break;
    case 12:
    {
        VectorizedMemset(&data_020fdc20.currentEntry->unk_2c.type12, 
            0, sizeof(data_020fdc20.currentEntry->unk_2c.type12));
        data_020fdc20.currentEntry->unk_2c.type12.subtype = (params++)->ToInt();
        data_020fdc20.currentEntry->unk_2c.type12.unk_1 = (params++)->ToInt();
        data_020fdc20.currentEntry->unk_2c.type12.unk_2 = (params++)->ToInt();

        // why didn't we do this 200 lines ago...
        ZoneFeatures::Opcode6aEntry* entry = data_020fdc20.currentEntry;
        switch (entry->unk_2c.type12.subtype)
        {
        case 1:
            entry->unk_2c.type12.subtype1.unk_0 = 0;
            entry->unk_2c.type12.subtype1.unk_1 = 0;
            entry->unk_2c.type12.subtype1.unk_3 = 0;
            entry->unk_2c.type12.subtype1.unk_4 = (params++)->ToInt();
            entry->unk_2c.type12.subtype1.unk_5 = (params++)->ToInt();
            entry->unk_2c.type12.subtype1.unk_2 = (params++)->ToInt();
            break;
        case 2:
            entry->unk_2c.type12.subtype2.unk_0 = 0;
            entry->unk_2c.type12.subtype2.unk_4 = 0;
            entry->unk_2c.type12.subtype2.unk_8 = 4096.0f * (params++)->ToFloat();
            entry->unk_2c.type12.subtype2.unk_c = 4096.0f * (params++)->ToFloat();
            entry->unk_2c.type12.subtype2.unk_10 = 25;
            break;
        case 3:
            entry->unk_2c.type12.subtype3.unk_0 = 0;
            entry->unk_2c.type12.subtype3.unk_1 = 0;
            entry->unk_2c.type12.subtype3.unk_4 = 4096 * 9 / 20; // 0.45
            entry->unk_2c.type12.subtype3.unk_8 = 4096.0f * (params++)->ToFloat();
            entry->unk_2c.type12.subtype3.unk_c = 4096.0f * (params++)->ToFloat();
            entry->unk_2c.type12.subtype3.unk_10 = 4096.0f * (params++)->ToFloat();
            entry->unk_2c.type12.subtype3.unk_8 = 4096 * 9 / 20; // 0.45
            entry->unk_2c.type12.subtype3.unk_10 = -4096 / 10; // -0.1
            break;
        }
        break;
    }
    }
    return 1;
}

int WarpScript_Opcode_7b(Script::Parameter* params, int numParams)
{
    int count = params[0].ToInt();
    ZoneFeatures::Opcode7bEntry* alloc = 
        (ZoneFeatures::Opcode7bEntry*)data_020fdc20.allocator->Allocate(count * sizeof(ZoneFeatures::Opcode7bEntry));
    if (alloc == NULL)
        return 0;
    data_020fdc20.warp->SetOpcode7bAllocation(alloc, count);
    return 1;
}

int WarpScript_Opcode_7c(Script::Parameter* params, int numParams)
{
    ZoneFeatures::Opcode7bEntry entry;
    params = params->ToVec3fix(&entry.vector_0.vec);
    params = params->ToVec3fix(&entry.vector_c.vec);
    entry.unk_18 = 4096.0f * (params++)->ToFloat();

    fix32_t mhalfx = -entry.vector_c.vec.x / 2;
    fix32_t mhalfz = -entry.vector_c.vec.z / 2;
    fix32_t phalfx = entry.vector_c.vec.x / 2;
    fix32_t phalfz = entry.vector_c.vec.z / 2;
    

    Vector3fix vertices[4] = { 0 };

    vertices[0].x = mhalfx;
    vertices[0].z = mhalfz;
    vertices[1].x = mhalfx;
    vertices[1].z = phalfz;
    vertices[2].x = phalfx;
    vertices[2].z = mhalfz;
    vertices[3].x = phalfx;
    vertices[3].z = phalfz;

    Matrix4x3 matrix;
    matrix = RotationMatrixY(entry.unk_18);

    for (int i = 0; i < 4; i++)
        Mat4x3_ApplyToVector(&vertices[i], &matrix, &vertices[i]);

    entry.minX = vertices[0].x;
    entry.minZ = vertices[0].z;
    entry.maxX = vertices[0].x;
    entry.maxZ = vertices[0].z;

    for (int i = 1; i < 4; i++)
    {
        if (entry.minX > vertices[i].x)
            entry.minX = vertices[i].x;
        if (entry.minZ > vertices[i].z)
            entry.minZ = vertices[i].z;
        if (entry.maxX < vertices[i].x)
            entry.maxX = vertices[i].x;
        if (entry.maxZ < vertices[i].z)
            entry.maxZ = vertices[i].z;
    }

    entry.minX += entry.vector_0.vec.x;
    entry.minZ += entry.vector_0.vec.z;
    entry.maxX += entry.vector_0.vec.x;
    entry.maxZ += entry.vector_0.vec.z;

    Struct_020fdc20* instance = &data_020fdc20;
    instance++;
    (instance - 1)->warp->CreateOpcode7bEntry(entry);
    return 1;
}

void ExecuteZoneWarpScript(const void* data, unsigned int length, ZoneFeatures* warp, SafeAllocator* alloc)
{
    if (length == 0)
        return;
    data_020fdc20.warp = warp;
    data_020fdc20.allocator = alloc;
    data_020fdc20.currentEntry = NULL;
    
    Script script;
    script.Initialize();
    script.SetOpcodeLookup(data_020ef388);
    script.Load(data, length);
    script.Execute();

    data_020fdc20.warp = NULL;
    data_020fdc20.allocator = NULL;
    data_020fdc20.currentEntry = NULL;
}

void ZoneFeatures::Reset()
{
    VectorizedMemset(this, 0, sizeof(ZoneFeatures));
}

void ZoneFeatures::AllocateOpcode64Entries(int count, SafeAllocator *alloc)
{
    entries64_ = (Opcode64Entry*)alloc->Allocate(count * sizeof(Opcode64Entry));
    arraySize64_ = 0;
    arrayCapacity64_ = count;
}

void ZoneFeatures::AllocateGrottoTileFeaturePlacementEntries(int count, SafeAllocator* alloc)
{
    grottoTileFeatureEntries_ = (TileFeaturePlacementData*)alloc->Allocate(count * sizeof(TileFeaturePlacementData));
    grottoTileFeatureEntryCount_ = 0;
    grottoTileFeatureEntryCapacity_ = count;
}

void ZoneFeatures::LoadFromScript(SafeAllocator* alloc, const void* script, unsigned int length)
{
    ExecuteZoneWarpScript(script, length, this, alloc);
}

ZoneFeatures::Opcode64Entry* ZoneFeatures::GetOpcode64Entry(int index)
{
    if (index < 0 || arraySize64_ <= index)
        return NULL;
    return &entries64_[index];
}

void ZoneFeatures::CreateOpcode64Entry(const Opcode64Entry& source)
{
    if (arraySize64_ >= arrayCapacity64_)
        return;
    Opcode64Entry& dest = entries64_[arraySize64_];

    dest.unk_0 = source.unk_0;
    dest.vector_4 = source.vector_4;
    COPY_ARRAY(dest.string_10, source.string_10);
    COPY_ARRAY(dest.string_20, source.string_20);
    COPY_ARRAY(dest.string_30, source.string_30);
    COPY_ARRAY(dest.unknown_40, source.unknown_40);
    arraySize64_++;
}

void ZoneFeatures::CreateGrottoTileFeaturePlacementEntry(const TileFeaturePlacementData& source)
{
    if (grottoTileFeatureEntryCount_ >= grottoTileFeatureEntryCapacity_)
        return;

    TileFeaturePlacementData& dest = grottoTileFeatureEntries_[grottoTileFeatureEntryCount_];
    dest.tileID = source.tileID;
    COPY_ARRAY(dest.directionBitmasks, source.directionBitmasks);
    COPY_ARRAY(dest.tilename, source.tilename);

    grottoTileFeatureEntryCount_++;
}

TileFeaturePlacementData *ZoneFeatures::GetGrottoTileFeaturePlacementEntry(const char *name)
{
    TileFeaturePlacementData* loopEntry = this->grottoTileFeatureEntries_;
    for (int i = 0; i < grottoTileFeatureEntryCount_; i++, loopEntry++)
    {
        if (strstr(loopEntry->tilename, name))
            return loopEntry;
    }
    return NULL;
}

void ZoneFeatures::AllocateOpcode66Entries(int count, SafeAllocator *alloc)
{
    entries66_ = (Opcode66Entry*)alloc->Allocate(count * sizeof(Opcode66Entry));
    arraySize66_ = 0;
    arrayCapacity66_ = count;
}

void ZoneFeatures::CreateOpcode66Entry(const Opcode66Entry& source)
{
    if (arraySize66_ >= arrayCapacity66_)
        return;
    Opcode66Entry& dest = entries66_[arraySize66_];

    COPY_ARRAY(dest.unk_0, source.unk_0);
    COPY_ARRAY(dest.unk_18, source.unk_18);
    dest.unk_20 = source.unk_20;

    arraySize66_++;
}

void ZoneFeatures::AllocateOpcode68Entries(int count, SafeAllocator* alloc)
{
    entries68_ = (Opcode68Entry*)alloc->Allocate(count * sizeof(Opcode68Entry));
    arraySize68_ = 0;
    arrayCapacity68_ = count;
}

void ZoneFeatures::CreateOpcode68Entry(const Opcode68Entry& source) 
{
    if (arraySize68_ >= arrayCapacity68_)
        return;
    Opcode68Entry& dest = entries68_[arraySize68_];

    dest.unk_0 = source.unk_0;
    COPY_ARRAY(dest.unk_4, source.unk_4);
    dest.unk_1c = source.unk_1c;
    COPY_ARRAY(dest.unk_20, source.unk_20);
    dest.unk_50 = source.unk_50;
    dest.unk_52 = source.unk_52;
    dest.unk_54 = source.unk_54;
    COPY_ARRAY(dest.unk_58, source.unk_58);
    COPY_ARRAY(dest.unk_64, source.unk_64);
    dest.unk_6c = source.unk_6c;

    arraySize68_++;
}

void ZoneFeatures::AllocateOpcode6aEntries(int count, SafeAllocator *alloc)
{
    entries6a_ = (Opcode6aEntry*)alloc->Allocate(count * sizeof(Opcode6aEntry));
    arraySize6a_ = 0;
    arrayCapacity6a_ = count;
}

ZoneFeatures::Opcode6aEntry* ZoneFeatures::CreateOpcode6aEntry(const Opcode6aEntry& source) 
{
    if (arraySize6a_ < arrayCapacity6a_)
    {
        Opcode6aEntry& dest = entries6a_[arraySize6a_];
        entries6a_[arraySize6a_].unk_0 = source.unk_0;
        dest.maybeType = source.maybeType;
        dest.unk_8 = source.unk_8;
        dest.unk_14 = source.unk_14;
        dest.unk_20 = source.unk_20;
        dest.unk_22 = source.unk_22;
        dest.unk_24 = source.unk_24;
        dest.unk_28 = source.unk_28;
        dest.unk_2c = source.unk_2c;
        dest.pNext = source.pNext;

        int cacheIdx = entries6a_[arraySize6a_].maybeType;

        if (entries6aByType_[cacheIdx] == NULL)
            entries6aByType_[cacheIdx] = &entries6a_[arraySize6a_];
        else
        {
            entries6a_[arraySize6a_].pNext = entries6aByType_[cacheIdx];
            entries6aByType_[cacheIdx] = &entries6a_[arraySize6a_];
        }
        
        arraySize6a_++;
        return &dest;
    }
    return NULL;
}

void ZoneFeatures::SetOpcode7bAllocation(Opcode7bEntry* array, unsigned short capacity)
{
    entries7b_ = array;
    arrayCapacity7b_ = capacity;
    arraySize7b_ = 0;
}

void ZoneFeatures::CreateOpcode7bEntry(const Opcode7bEntry& source)
{
    if (entries7b_ == NULL)
        return;

    Opcode7bEntry& dest = entries7b_[arraySize7b_++];
    dest.vector_0 = source.vector_0;
    dest.vector_c = source.vector_c;
    dest.unk_18 = source.unk_18;
    dest.minX = source.minX;
    dest.minZ = source.minZ;
    dest.maxX = source.maxX;
    dest.maxZ = source.maxZ;
}