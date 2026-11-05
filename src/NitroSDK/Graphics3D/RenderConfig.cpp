#include "NitroSDK/Graphics3D/RenderConfig.h"
#include "NitroSDK/Graphics3D/Graphics3D.h"
#include <globaldefs.h>

Nitro3DRenderConfig g_nitro3DRenderConfig;

#define BUILD_LIGHT_VECTOR(idx, x, y, z) (((unsigned)(idx) << 30) | ((unsigned)(z) << 20) | ((unsigned)(y) << 10) | ((unsigned)(x)))
#define RGB(r,g,b) (((unsigned)(r)) | ((unsigned)(g) << 5) | ((unsigned)(b) << 10))
#define BUILD_LIGHT_COLOR(idx, r,g,b) (((unsigned)(idx) << 30) | RGB((r),(g),(b)))

// kind of evil but this prevents invoking Vector3fix::operator= / C++-style copy assignment
static inline void ArrayCopyVec3(void* dst, const void* src)
{
    struct ArrayVec3 { fix32_t values[3]; };
    *(ArrayVec3*)dst = *(const ArrayVec3*)src;
}

void Nitro3DRenderConfig::Reset()
{
    g_nitro3DRenderConfig.magicCommands_0 = COMBINE_GXFIFO_COMMANDS4(
        GXFifoCommand_SetMatrixMode,
        GXFifoCommand_LoadMat4x4,
        GXFifoCommand_SetMatrixMode,
        GXFifoCommand_LoadMat4x3
    );

    g_nitro3DRenderConfig.magicParam_4 = 0;
    g_nitro3DRenderConfig.magicParam_48 = 2;

    g_nitro3DRenderConfig.magicCommands_7c = COMBINE_GXFIFO_COMMANDS4(
        GXFifoCommand_SetLightVector,
        GXFifoCommand_SetLightVector,
        GXFifoCommand_SetLightVector,
        GXFifoCommand_SetLightVector
    );

    g_nitro3DRenderConfig.magicCommands_90 = COMBINE_GXFIFO_COMMANDS4(
        GXFifoCommand_DiffuseAmbientReflect,
        GXFifoCommand_SpecularReflectEmit,
        GXFifoCommand_SetPolygonAttr,
        GXFifoCommand_SetViewport
    );

    g_nitro3DRenderConfig.magicCommands_a4 = COMBINE_GXFIFO_COMMANDS4(
        GXFifoCommand_SetLightColor,
        GXFifoCommand_SetLightColor,
        GXFifoCommand_SetLightColor,
        GXFifoCommand_SetLightColor
    );

    g_nitro3DRenderConfig.magicCommands_b8 = COMBINE_GXFIFO_COMMANDS3(
        GXFifoCommand_MultiplyMat4x3,
        GXFifoCommand_ScaleMatrix,
        GXFifoCommand_SetTexImageParams
    );

    Mat4x3_WriteIdentity(&g_nitro3DRenderConfig.viewMatrix);
    Mat4x4_WriteIdentity(&g_nitro3DRenderConfig.projectionMatrix);

    uint32_t minusInvSqrt3 = 0x2d8;
    g_nitro3DRenderConfig.lightVectors[0] = BUILD_LIGHT_VECTOR(0,
        minusInvSqrt3, minusInvSqrt3, minusInvSqrt3);
    g_nitro3DRenderConfig.lightVectors[1] = BUILD_LIGHT_VECTOR(1, 0x200, 0, 0);
    g_nitro3DRenderConfig.lightVectors[2] = BUILD_LIGHT_VECTOR(2, 0x1ff, 0, 0);
    g_nitro3DRenderConfig.lightVectors[3] = BUILD_LIGHT_VECTOR(3, 0, 0x200, 0);

    g_nitro3DRenderConfig.diffuseAmbientArg = (RGB(16, 16, 16) << 16) | (1 << 15) | RGB(16, 16, 16);
    g_nitro3DRenderConfig.specularArg = (RGB(16, 16, 16) << 16) | (1 << 15) | RGB(16, 16, 16);
    // alpha = 31 | render forward-facing polygons | enable all lights
    g_nitro3DRenderConfig.polygonAttrArg = (0x1f << 16) | (1 << 7) | 0xf;
    // left = 0 | top = 0 | right = 255 | bottom = 191
    g_nitro3DRenderConfig.viewportArg = 0 | (0 << 8) | (255 << 16) | (191 << 24);
    g_nitro3DRenderConfig.lightColors[0] = BUILD_LIGHT_COLOR(0, 31, 31, 31);
    g_nitro3DRenderConfig.lightColors[1] = BUILD_LIGHT_COLOR(1, 31, 0, 0);
    g_nitro3DRenderConfig.lightColors[2] = BUILD_LIGHT_COLOR(2, 0, 31, 0);
    g_nitro3DRenderConfig.lightColors[3] = BUILD_LIGHT_COLOR(3, 0, 0, 31);

    g_nitro3DRenderConfig.objectRotationPosition.translation.x = 0;
    g_nitro3DRenderConfig.objectRotationPosition.translation.y = 0;
    g_nitro3DRenderConfig.objectRotationPosition.translation.z = 0;

    Mat3x3_WriteIdentity(&g_nitro3DRenderConfig.objectRotationPosition.rotation);

    g_nitro3DRenderConfig.objectScale.x = 1 << 12;
    g_nitro3DRenderConfig.objectScale.y = 1 << 12;
    g_nitro3DRenderConfig.objectScale.z = 1 << 12;

    g_nitro3DRenderConfig.texImageParams = 0;
    g_nitro3DRenderConfig.flags = 0;

    g_nitro3DRenderConfig.eyeVector.x = g_nitro3DRenderConfig.eyeVector.y = g_nitro3DRenderConfig.eyeVector.z = 0;
    g_nitro3DRenderConfig.upVector.x = g_nitro3DRenderConfig.upVector.z = 0;
    g_nitro3DRenderConfig.upVector.y = 1 << 12;

    g_nitro3DRenderConfig.targetVector.x = g_nitro3DRenderConfig.targetVector.y = 0;
    g_nitro3DRenderConfig.targetVector.z = -1 << 12;
}

void Nitro3DRenderConfig::SubmitToFifo()
{
    Nitro3DRenderConfig* instance = &g_nitro3DRenderConfig;
    uint32_t* commands = 
#if defined(usa)
    (uint32_t*)0x0210a010; // i hate this
#elif defined(jpn)
    (uint32_t*)0x02109cc8;
#else
#error need to hardcode address of RenderConfig instance
#endif
    Nitro3Di_SendFifoCommand(instance->magicCommands_0, commands + 1, 0x3e);

    instance->flags &= ~(1 << RENDER_CONFIG_FLAG_0);
    instance->flags &= ~(1 << RENDER_CONFIG_FLAG_1);
}

void Nitro3DRenderConfig::SetObjectPosition(Vector3fix *pos)
{
    if (pos != NULL)
    {
        ArrayCopyVec3(&g_nitro3DRenderConfig.objectRotationPosition.translation, pos);
        g_nitro3DRenderConfig.flags &= ~((1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID) | (1 << RENDER_CONFIG_FLAG_5) | (1 << RENDER_CONFIG_FLAG_2));
    }
}

void Nitro3DRenderConfig::SetObjectScale(Vector3fix *scale)
{
    if (scale != NULL)
    {
        ArrayCopyVec3(&g_nitro3DRenderConfig.objectScale, scale);
        g_nitro3DRenderConfig.flags &= ~((1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID) | (1 << RENDER_CONFIG_FLAG_5) | (1 << RENDER_CONFIG_FLAG_2));
    }
}

void Nitro3DRenderConfig::SetLightVector(unsigned int light, fix16_t x, fix16_t y, fix16_t z)
{
    uint32_t formattedX = (x >> 3) & 0x3ff;
    uint32_t formattedY = ((uint32_t)(y >> 3) << 22) >> 12;
    uint32_t formattedZ = ((uint32_t)(z >> 3) << 22) >> 2;
    g_nitro3DRenderConfig.lightVectors[light] = formattedX | formattedY | formattedZ | (light << 30);
}

void Nitro3DRenderConfig::SetLightColor(unsigned int light, uint32_t rgb)
{
    g_nitro3DRenderConfig.lightColors[light] = rgb | (light << 30);
}

void Nitro3DRenderConfig::SetDiffuseAmbientColors(uint32_t diffuseRGB, uint32_t ambientRGB, int setVertexColor)
{
    setVertexColor = (setVertexColor ? 1 : 0);
    g_nitro3DRenderConfig.diffuseAmbientArg = diffuseRGB | (ambientRGB << 16) | (setVertexColor << 15);
}

void Nitro3DRenderConfig::SetPolygonAttributes(unsigned int lights, unsigned int mode,
    unsigned int culling, unsigned int polygonID, unsigned int alpha, unsigned int miscFlags)
{
    g_nitro3DRenderConfig.polygonAttrArg = miscFlags | 
        (lights | mode << 4 | culling << 6) | polygonID << 24 | alpha << 16;
}

const Matrix4x3* Nitro3DRenderConfig::GetInverseViewMatrix()
{
    if (!(g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_INVERSE_VIEW_CACHE_VALID)))
    {
        Mat4x3_Invert(&g_nitro3DRenderConfig.viewMatrix, &g_nitro3DRenderConfig.cachedInverseView);
        g_nitro3DRenderConfig.flags |= (1 << RENDER_CONFIG_FLAG_INVERSE_VIEW_CACHE_VALID);
    }
    return &g_nitro3DRenderConfig.cachedInverseView;
}

static void RecalculateWorldViewAndInverse()
{
    Mat4x3_Multiply(&g_nitro3DRenderConfig.objectRotationPosition, &g_nitro3DRenderConfig.viewMatrix, &g_nitro3DRenderConfig.cachedWorldView);
    Mat4x3_ApplyScale(&g_nitro3DRenderConfig.cachedWorldView, &g_nitro3DRenderConfig.cachedWorldView,
        g_nitro3DRenderConfig.objectScale.x, g_nitro3DRenderConfig.objectScale.y, g_nitro3DRenderConfig.objectScale.z);
    Mat4x3_Invert(&g_nitro3DRenderConfig.cachedWorldView, &g_nitro3DRenderConfig.cachedInverseWorldView);
}

const Matrix4x3* Nitro3DRenderConfig::GetCombinedWorldViewMatrix()
{
    if (!(g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID)))
    {
        RecalculateWorldViewAndInverse();
        g_nitro3DRenderConfig.flags |= (1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID);
    }
    return &g_nitro3DRenderConfig.cachedWorldView;
}

const Matrix4x3* Nitro3DRenderConfig::GetInverseCombinedWorldViewMatrix()
{
    if (!(g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID)))
    {
        RecalculateWorldViewAndInverse();
        g_nitro3DRenderConfig.flags |= (1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID);
    }
    return &g_nitro3DRenderConfig.cachedInverseWorldView;
}

void Nitro3DRenderConfig::GetViewport(int *left, int *top, int *right, int *bottom)
{
    if (left != NULL)
        *left = g_nitro3DRenderConfig.viewportArg & 0xff;
    if (top != NULL)
        *top = (g_nitro3DRenderConfig.viewportArg >> 8) & 0xff;
    if (right != NULL)
        *right = (g_nitro3DRenderConfig.viewportArg >> 16) & 0xff;
    if (bottom != NULL)
        *bottom = (g_nitro3DRenderConfig.viewportArg >> 24) & 0xff;
}