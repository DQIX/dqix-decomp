#pragma once

#include "std_library_functions.h"
#include "../Vector.h"

// Functionally similar to a uniform buffer in OpenGL etc., this provides 
// various parameters to configure 3D rendering, notably world/view/projection
// matrices and lighting settings. The data is interspersed with various magic
// constants so that you can just send the whole thing over to the geometry
// FIFO prior to rendering a model.
struct RenderConfig
{
    // holds 0x17101610 (set matrix mode, load 4x4, set matrix mode, load 4x3)
    uint32_t magicCommands_0;
    // holds 0 (parameter to set matrix mode, switches to projection mode)
    uint32_t magicParam_4;
    
    Matrix4x4 projectionMatrix;

    // holds 2 (parameter to set matrix mode, switches to position+direction mode)
    uint32_t magicParam_48;

    // the view matrix is the *inverse* of the camera's world matrix, i.e. it
    // transforms world coordinates into coordinates local to the camera. When
    // this is the identity, the camera is positioned at the origin and facing
    // in the negative z-direction
    Matrix4x3 viewMatrix;

    // holds 0x32323232 (4x set light vector)
    uint32_t magicCommands_7c;

    // Each parameter describes the direction of one of the 4 lights, packed
    // as follows: x in bits 0-9, y in bits 10-19, z in bits 20-29, and light
    // index in bits 30-31. Each of x, y, z is fixed point with 9 positions
    // after the decimal point (i.e. 0x200 holds the minimum value of -1 and
    // 0x1ff holds the maximum value of 0.998ish)
    uint32_t lightVectors[4];

    // holds 0x60293130 (set diffuse/ambient colors, set specular colors, set
    // polygon attributes, set viewport)
    uint32_t magicCommands_90;

    uint32_t diffuseAmbientArg;
    uint32_t specularArg;
    // note materials have their own argument and a mask for POLYGON_ATTR, this
    // specifies the values in positions where the mask is zero
    uint32_t polygonAttrArg;
    uint32_t viewportArg;

    // holds 0x33333333 (4x set light color)
    uint32_t magicCommands_a4;

    // Each parameter holds the color of one of the 4 lights, packed as follows:
    // bits 0-4 hold red, 5-9 hold green, 10-14 hold blue, and bits 30-31 hold
    // the light index
    uint32_t lightColors[4];

    // holds 0x2a1b19 (multiply by 4x3, multiply by scale matrix, set teximage params)
    uint32_t magicCommands_b8;

    // you can safely combine these into a 4x3 matrix representing rotation->translation
    Matrix4x3 objectRotationPosition;

    Vector3fix objectScale;

    uint32_t texImageParams;

    uint32_t flags;

    // inverse of the camera's view matrix, i.e. the world matrix of the camera.
    // Cached to minimize matrix inversion calls. Bit 3 of flags is a dirty flag
    // for this matrix, when *clear* the matrix needs to be recalculated
    Matrix4x3 cachedInverseView;

    // holds the combined object world matrix and view matrix.
    // Cached to minimize function calls. Bit 7 of flags is a dirty flag for 
    // this matrix, when *clear* the matrix needs to be recalculated
    Matrix4x3 cachedWorldView;

    // holds the inverse of the combined world+view matrix.
    // Cached to minimize function calls, and recomputed alongside 
    // cachedWorldView (with bit 7 of flags being the dirty flag again)
    Matrix4x3 cachedInverseWorldView;

    char unk_190[0xb0];

    Vector3fix eyeVector;
    Vector3fix upVector;
    Vector3fix targetVector;

    static void Reset();

    static void SubmitToFifo();
    static void SetObjectPosition(Vector3fix* pos);
    static void SetObjectScale(Vector3fix* scale);

    // Accepts actual 1.19.12 fixed point numbers, the function will transform
    // them to the required format for FIFO submission
    static void SetLightVector(unsigned int light, fix16_t x, fix16_t y, fix16_t z);

    // RGB is in the usual 5b.5g.5r format
    static void SetLightColor(unsigned int light, uint32_t rgb);

    // setVertexColor is a boolean which, if set, causes the diffuse color to be
    // used as the vertex color
    static void SetDiffuseAmbientColors(uint32_t diffuseRGB, uint32_t ambientRGB, int setVertexColor);

    static void SetPolygonAttributes(unsigned int lights, unsigned int mode,
        unsigned int culling, unsigned int polygonID, unsigned int alpha,
        unsigned int miscFlags);

    static const Matrix4x3* GetInverseViewMatrix();

    // note: matrix does not account for object scale
    static const Matrix4x3* GetCombinedWorldViewMatrix();

    // note: matrix does not account for object scale
    static const Matrix4x3* GetInverseCombinedWorldViewMatrix();

    static void GetViewport(int* left, int* top, int* right, int* bottom);
};

#define RENDER_CONFIG_FLAG_0 0
#define RENDER_CONFIG_FLAG_1 1
#define RENDER_CONFIG_FLAG_2 2
#define RENDER_CONFIG_FLAG_INVERSE_VIEW_CACHE_VALID 3
#define RENDER_CONFIG_FLAG_4 4
#define RENDER_CONFIG_FLAG_5 5
#define RENDER_CONFIG_FLAG_6 6
#define RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID 7

#if defined(jpn)
#define data_0210a010 data_02109cc8
#endif

extern RenderConfig data_0210a010;