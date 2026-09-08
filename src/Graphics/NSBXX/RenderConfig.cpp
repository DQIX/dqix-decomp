#include "Graphics/NSBXX/RenderConfig.h"
#include "Graphics/NSBXX/GeometryFifo.h"
#include <globaldefs.h>

#pragma optimize_for_size off

#define BUILD_LIGHT_VECTOR(idx, x, y, z) (((unsigned)(idx) << 30) | ((unsigned)(z) << 20) | ((unsigned)(y) << 10) | ((unsigned)(x)))
#define RGB(r,g,b) (((unsigned)(r)) | ((unsigned)(g) << 5) | ((unsigned)(b) << 10))
#define BUILD_LIGHT_COLOR(idx, r,g,b) (((unsigned)(idx) << 30) | RGB((r),(g),(b)))

// kind of evil but this prevents invoking Vector3fix::operator=
static inline void ArrayCopyVec3(void* dst, const void* src)
{
    struct ArrayVec3 { fix32_t values[3]; };
    *(ArrayVec3*)dst = *(const ArrayVec3*)src;
}

void RenderConfig::Reset()
{
    data_0210a010.magicCommands_0 = COMBINE_GXFIFO_COMMANDS4(
        GXFifoCommand_SetMatrixMode,
        GXFifoCommand_LoadMat4x4,
        GXFifoCommand_SetMatrixMode,
        GXFifoCommand_LoadMat4x3
    );

    data_0210a010.magicParam_4 = 0;
    data_0210a010.magicParam_48 = 2;

    data_0210a010.magicCommands_7c = COMBINE_GXFIFO_COMMANDS4(
        GXFifoCommand_SetLightVector,
        GXFifoCommand_SetLightVector,
        GXFifoCommand_SetLightVector,
        GXFifoCommand_SetLightVector
    );

    data_0210a010.magicCommands_90 = COMBINE_GXFIFO_COMMANDS4(
        GXFifoCommand_DiffuseAmbientReflect,
        GXFifoCommand_SpecularReflectEmit,
        GXFifoCommand_SetPolygonAttr,
        GXFifoCommand_SetViewport
    );

    data_0210a010.magicCommands_a4 = COMBINE_GXFIFO_COMMANDS4(
        GXFifoCommand_SetLightColor,
        GXFifoCommand_SetLightColor,
        GXFifoCommand_SetLightColor,
        GXFifoCommand_SetLightColor
    );

    data_0210a010.magicCommands_b8 = COMBINE_GXFIFO_COMMANDS3(
        GXFifoCommand_MultiplyMat4x3,
        GXFifoCommand_ScaleMatrix,
        GXFifoCommand_SetTexImageParams
    );

    Mat4x3_WriteIdentity(&data_0210a010.viewMatrix);
    Mat4x4_WriteIdentity(&data_0210a010.projectionMatrix);

    uint32_t minusInvSqrt3 = 0x2d8;
    data_0210a010.lightVectors[0] = BUILD_LIGHT_VECTOR(0,
        minusInvSqrt3, minusInvSqrt3, minusInvSqrt3);
    data_0210a010.lightVectors[1] = BUILD_LIGHT_VECTOR(1, 0x200, 0, 0);
    data_0210a010.lightVectors[2] = BUILD_LIGHT_VECTOR(2, 0x1ff, 0, 0);
    data_0210a010.lightVectors[3] = BUILD_LIGHT_VECTOR(3, 0, 0x200, 0);

    data_0210a010.diffuseAmbientArg = (RGB(16, 16, 16) << 16) | (1 << 15) | RGB(16, 16, 16);
    data_0210a010.specularArg = (RGB(16, 16, 16) << 16) | (1 << 15) | RGB(16, 16, 16);
    // alpha = 31 | render forward-facing polygons | enable all lights
    data_0210a010.polygonAttrArg = (0x1f << 16) | (1 << 7) | 0xf;
    // left = 0 | top = 0 | right = 255 | bottom = 191
    data_0210a010.viewportArg = 0 | (0 << 8) | (255 << 16) | (191 << 24);
    data_0210a010.lightColors[0] = BUILD_LIGHT_COLOR(0, 31, 31, 31);
    data_0210a010.lightColors[1] = BUILD_LIGHT_COLOR(1, 31, 0, 0);
    data_0210a010.lightColors[2] = BUILD_LIGHT_COLOR(2, 0, 31, 0);
    data_0210a010.lightColors[3] = BUILD_LIGHT_COLOR(3, 0, 0, 31);

    data_0210a010.objectRotationPosition.translation.x = 0;
    data_0210a010.objectRotationPosition.translation.y = 0;
    data_0210a010.objectRotationPosition.translation.z = 0;

    Mat3x3_WriteIdentity(&data_0210a010.objectRotationPosition.rotation);

    data_0210a010.objectScale.x = 1 << 12;
    data_0210a010.objectScale.y = 1 << 12;
    data_0210a010.objectScale.z = 1 << 12;

    data_0210a010.texImageParams = 0;
    data_0210a010.flags = 0;

    data_0210a010.eyeVector.x = data_0210a010.eyeVector.y = data_0210a010.eyeVector.z = 0;
    data_0210a010.upVector.x = data_0210a010.upVector.z = 0;
    data_0210a010.upVector.y = 1 << 12;

    data_0210a010.targetVector.x = data_0210a010.targetVector.y = 0;
    data_0210a010.targetVector.z = -1 << 12;
}

void RenderConfig::SubmitToFifo()
{
    RenderConfig* instance = &data_0210a010;
    uint32_t* commands = 
#if defined(usa)
    (uint32_t*)0x0210a010; // i hate this
#elif defined(jpn)
    (uint32_t*)0x02109cc8;
#else
#error need to hardcode address of RenderConfig instance
#endif
    SubmitCommandToGeometryFifo(instance->magicCommands_0, commands + 1, 0x3e);

    instance->flags &= ~(1 << RENDER_CONFIG_FLAG_0);
    instance->flags &= ~(1 << RENDER_CONFIG_FLAG_1);
}

void RenderConfig::SetObjectPosition(Vector3fix *pos)
{
    if (pos != NULL)
    {
        ArrayCopyVec3(&data_0210a010.objectRotationPosition.translation, pos);
        data_0210a010.flags &= ~((1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID) | (1 << RENDER_CONFIG_FLAG_5) | (1 << RENDER_CONFIG_FLAG_2));
    }
}

void RenderConfig::SetObjectScale(Vector3fix *scale)
{
    if (scale != NULL)
    {
        ArrayCopyVec3(&data_0210a010.objectScale, scale);
        data_0210a010.flags &= ~((1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID) | (1 << RENDER_CONFIG_FLAG_5) | (1 << RENDER_CONFIG_FLAG_2));
    }
}

void RenderConfig::SetLightVector(unsigned int light, fix16_t x, fix16_t y, fix16_t z)
{
    uint32_t formattedX = (x >> 3) & 0x3ff;
    uint32_t formattedY = ((uint32_t)(y >> 3) << 22) >> 12;
    uint32_t formattedZ = ((uint32_t)(z >> 3) << 22) >> 2;
    data_0210a010.lightVectors[light] = formattedX | formattedY | formattedZ | (light << 30);
}

void RenderConfig::SetLightColor(unsigned int light, uint32_t rgb)
{
    data_0210a010.lightColors[light] = rgb | (light << 30);
}

void RenderConfig::SetDiffuseAmbientColors(uint32_t diffuseRGB, uint32_t ambientRGB, int setVertexColor)
{
    setVertexColor = (setVertexColor ? 1 : 0);
    data_0210a010.diffuseAmbientArg = diffuseRGB | (ambientRGB << 16) | (setVertexColor << 15);
}

void RenderConfig::SetPolygonAttributes(unsigned int lights, unsigned int mode,
    unsigned int culling, unsigned int polygonID, unsigned int alpha, unsigned int miscFlags)
{
    data_0210a010.polygonAttrArg = miscFlags | 
        (lights | mode << 4 | culling << 6) | polygonID << 24 | alpha << 16;
}

const Matrix4x3* RenderConfig::GetInverseViewMatrix()
{
    if (!(data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_INVERSE_VIEW_CACHE_VALID)))
    {
        Mat4x3_Invert(&data_0210a010.viewMatrix, &data_0210a010.cachedInverseView);
        data_0210a010.flags |= (1 << RENDER_CONFIG_FLAG_INVERSE_VIEW_CACHE_VALID);
    }
    return &data_0210a010.cachedInverseView;
}

// can be made static later
void RecalculateWorldViewAndInverse()
{
    Mat4x3_Multiply(&data_0210a010.objectRotationPosition, &data_0210a010.viewMatrix, &data_0210a010.cachedWorldView);
    Mat4x3_ApplyScale(&data_0210a010.cachedWorldView, &data_0210a010.cachedWorldView,
        data_0210a010.objectScale.x, data_0210a010.objectScale.y, data_0210a010.objectScale.z);
    Mat4x3_Invert(&data_0210a010.cachedWorldView, &data_0210a010.cachedInverseWorldView);
}

const Matrix4x3* RenderConfig::GetCombinedWorldViewMatrix()
{
    if (!(data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID)))
    {
        RecalculateWorldViewAndInverse();
        data_0210a010.flags |= (1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID);
    }
    return &data_0210a010.cachedWorldView;
}

const Matrix4x3* RenderConfig::GetInverseCombinedWorldViewMatrix()
{
    if (!(data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID)))
    {
        RecalculateWorldViewAndInverse();
        data_0210a010.flags |= (1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID);
    }
    return &data_0210a010.cachedInverseWorldView;
}

void RenderConfig::GetViewport(int *left, int *top, int *right, int *bottom)
{
    if (left != NULL)
        *left = data_0210a010.viewportArg & 0xff;
    if (top != NULL)
        *top = (data_0210a010.viewportArg >> 8) & 0xff;
    if (right != NULL)
        *right = (data_0210a010.viewportArg >> 16) & 0xff;
    if (bottom != NULL)
        *bottom = (data_0210a010.viewportArg >> 24) & 0xff;
}