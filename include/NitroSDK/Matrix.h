#pragma once

#include "std_library_functions.h"

// Note: the convention seems to be that matrices act on *row vectors*
// by right multiplication, rather than the more common setting of
// acting on column vectors by left multiplication. In particular, we agree
// with typical DirectX conventions instead of OpenGL / math.
// Matrices are stored in row major order, i.e. each row of the matrix
// is contiguous in memory.
// GBATEK documentation is consistent with our choice, but the NSBMD
// docs are *not*. We choose to go with GBATEK specifically because certain
// SDK functions make more sense this way, e.g. matrix multiplication functions
// compute arg1 * arg2 and store it in arg3, but only if you view matrices
// as row major.
// As a result of this choice,
//   - A*B represents the transformation obtained by doing A first, then B
//   - Affine transformations have rightmost column (0, 0, 0, 1)^T and can
//     thus be represented concisely as a 4x3 matrix
//   - A translation matrix by (u, v, w) has bottom row equal to (u, v, w, 1)

// Fixed-point number with 12 bits after the point
typedef int32_t fix32_t;
// Fixed-point number with 12 bits after the point
typedef int16_t fix16_t;

#define FIX32_MULTIPLY(a, b) (fix32_t)((((int64_t)(a) * (int64_t)(b)) + (int64_t)0x800) >> (int64_t)12)
#define FIX32_MULTIPLY_SIMPLE(a, b) (fix32_t)(((int64_t)(a) * (int64_t)(b)) >> 12)

#define FIX32_MULTIPLY_v2(a, b) ((((int64_t)(unsigned int)(b) * (int64_t)(a))) >> (int64_t)12) // no cast to fix32_t
#define FIX32_MULTIPLY_v3(a, b) ((((int64_t)(b) * (int64_t)(a))) >> (int64_t)12)

#define FIX32_MULTIPLY_v4(a, b) ((((int64_t)(unsigned)(b) * (int64_t)(signed)(a))) >> (int64_t)12) // no cast to fix32_t

typedef struct Vector3i
{
    int32_t x;
    int32_t y;
    int32_t z;
} Vector3i;

typedef struct Vector3s
{
    int16_t x;
    int16_t y;
    int16_t z;
} Vector3s;

typedef Vector3i Vector3fix;
typedef Vector3s Vector3fix16;

#define FIX_2PI 0x6488

typedef union Matrix3x3
{
    Vector3fix rows[3];
    fix32_t entries[9];
} Matrix3x3;

typedef union Matrix4x3
{
    Vector3fix rows[4];
    fix32_t entries[12];
    struct {
        Matrix3x3 rotation;
        Vector3fix translation;
    };
} Matrix4x3;

typedef union Vector4fix
{
    fix32_t entries[4];
    Vector3fix xyz;
    struct {
        fix32_t x;
        fix32_t y;
        fix32_t z;
        fix32_t w;
    };
};

typedef union Matrix4x4
{
    fix32_t entries[16];
    Vector4fix rows[4];
} Matrix4x4;

// These are probably nitro SDK functions, and some are handwritten in assembly.
// The others are a massive pain because of 64-bit, so I'm not matching them now
// but it should be useful for clarity to have them named.
#ifdef __cplusplus
extern "C"
{
#endif
    // usa: func_020c1180
    void Mat3x3_WriteIdentity(Matrix3x3* out);
    // usa: func_020c11a4
    // computes diag(x, y, z) * in and writes to out.
    // note the resulting matrix represents first scaling, then applying the
    // input matrix.
    void Mat3x3_ApplyScale(const Matrix3x3* in, Matrix3x3* out, fix32_t x, fix32_t y, fix32_t z);
    // usa: func_020c1264
    // writes an x-axis rotation into the matrix specified. Instead
    // of providing the angle of rotation, you specify the sine/cosine
    // values. The rotation is left-handed, i.e. the output is
    //      ( 1  0  0 )
    //  M = ( 0  c  s )
    //      ( 0 -s  c )
    void Mat3x3_WriteRotationX(Matrix3x3* out, fix32_t sine, fix32_t cosine);
    // usa: func_020c1280
    // writes a y-axis rotation into the matrix specified. Instead
    // of providing the angle of rotation, you specify the sine/cosine
    // values. The rotation is left-handed, i.e. the output is
    //     ( c  0 -s )
    // M = ( 0  1  0 )
    //     ( s  0  c )
    void Mat3x3_WriteRotationY(Matrix3x3* out, fix32_t sine, fix32_t cosine);
    // usa: func_020c129c
    // writes a z-axis rotation into the matrix specified. Instead
    // of providing the angle of rotation, you specify the sine/cosine
    // values. The rotation is left-handed, i.e. the output is
    //     (  c  s  0 )
    // M = ( -s  c  0 )
    //     (  0  0  1 )
    void Mat3x3_WriteRotationZ(Matrix3x3* out, fix32_t sine, fix32_t cosine);
    // usa: func_020c12b4
    // calculates the inverse of the input matrix.
    // returns 0 if the process succeeded (i.e. the input matrix was invertible)
    // and -1 if it failed.
    int Mat3x3_Invert(const Matrix3x3* in, Matrix3x3* out);
    // usa: func_020c15a4
    // computes inA * inB and stores to out.
    void Mat3x3_Multiply(const Matrix3x3* inA, const Matrix3x3* inB, Matrix3x3* out);
    // usa: func_020c17c4
    // computes inVec * inMat and stores to out.
    void Mat3x3_ApplyToVector(const Vector3fix* inVec, const Matrix3x3* inMat, Vector3fix* out);



    // usa: func_020c1840
    void Mat4x3_WriteIdentity(Matrix4x3* out);
    // usa: func_020c1868
    // converts an input 4x3 matrix to a 4x4 matrix.
    void Mat4x3_ConvertTo4x4(const Matrix4x3* in, Matrix4x4* out);
    // usa: func_020c189c
    // computes translate(x, y, z) * in, and writes to out.
    // note the resulting matrix represents first translating, then applying
    // the input matrix.
    void Mat4x3_ApplyTranslation(const Matrix4x3* in, Matrix4x3* out, fix32_t x, fix32_t y, fix32_t z);
    // usa: func_020c1948
    // computes diag(x, y, z) * in and writes to out.
    // note the resulting matrix represents first scaling, then applying the
    // input matrix.
    void Mat4x3_ApplyScale(const Matrix4x3* in, Matrix4x3* out, fix32_t x, fix32_t y, fix32_t z);
    // usa: func_020c197c
    // writes an x-axis rotation into the matrix specified. Instead
    // of providing the angle of rotation, you specify the sine/cosine
    // values. The rotation is left-handed, i.e. the output is
    //      ( 1  0  0 )
    //  M = ( 0  c  s )
    //      ( 0 -s  c )
    //      ( 0  0  0 )
    void Mat4x3_WriteRotationX(Matrix4x3* out, fix32_t sine, fix32_t cosine);
    // usa: func_020c199c
    // writes a y-axis rotation into the matrix specified. Instead
    // of providing the angle of rotation, you specify the sine/cosine
    // values. The rotation is left-handed, i.e. the output is
    //     ( c  0 -s )
    // M = ( 0  1  0 )
    //     ( s  0  c )
    //     ( 0  0  0 )
    void Mat4x3_WriteRotationY(Matrix4x3* out, fix32_t sine, fix32_t cosine);
    // usa: func_020c19b8
    // writes a z-axis rotation into the matrix specified. Instead
    // of providing the angle of rotation, you specify the sine/cosine
    // values. The rotation is left-handed, i.e. the output is
    //     (  c  s  0 )
    // M = ( -s  c  0 )
    //     (  0  0  1 )
    //     (  0  0  0 )
    void Mat4x3_WriteRotationZ(Matrix4x3* out, fix32_t sine, fix32_t cosine);
    // usa: func_020c19d4
    // calculates the inverse of the input matrix.
    // returns 0 if the process succeeded (i.e. the input matrix was invertible)
    // and -1 if it failed.
    int Mat4x3_Invert(const Matrix4x3* in, Matrix4x3* out);
    // usa: func_020c1d60
    // computes inA * inB and stores to out.
    void Mat4x3_Multiply(const Matrix4x3* inA, const Matrix4x3* inB, Matrix4x3* out);
    // usa: func_020c2034
    // computes inVec * inMat and stores to out.
    void Mat4x3_ApplyToVector(const Vector3fix* inVec, const Matrix4x3* inMat, Vector3fix* out);

    // usa: func_020c20d4
    // calculate a 4x3 view matrix
    void Mat4x3_WriteViewMatrix(const Vector3fix* eye, const Vector3fix* up, const Vector3fix* target, Matrix4x3* out);



    // usa: func_020c21dc
    void Mat4x4_WriteIdentity(Matrix4x4* out);
    // usa: func_020c2208
    // converts an input 4x4 matrix to a 4x3 matrix.
    void Mat4x4_ConvertTo4x3(const Matrix4x4* in, Matrix4x3* out);
    // usa: func_020c223c
    // computes inA * inB and stores to out.
    void Mat4x4_Multiply(const Matrix4x4* inA, const Matrix4x4* inB, Matrix4x4* out);
    // usa: func_020c28a0
    // I'm not entirely sure what this does, but the format looks similar to
    // e.g. glFrustum but doesn't match exactly
    void Mat4x4_MaybeWriteFrustum(fix32_t a, fix32_t b, fix32_t c, fix32_t d, fix32_t e, fix32_t f, Matrix4x4* out);
    // usa: func_020c29ec
    // Similar function to the previous, is used in places to populate the
    // RenderConfig's projection matrix, but I don't know what exactly it is
    // Might be something like glOrtho???
    // From call site in AtmosphericEffect it looks to be (top, bottom, left, right, near, far)
    // not sure about 7th parameter
    void Mat4x4_WriteProjectionUnknown(fix32_t a, fix32_t b, fix32_t c, fix32_t d, fix32_t e, fix32_t f, fix32_t g, Matrix4x4* out);


    // usa: func_020c2bf4
    // performs synchronous division of fixed-point numbers using the hardware divider
    fix32_t fix32_Divide(fix32_t num, fix32_t denom);
    // usa: func_020c2c04
    // calculates square root for a fixed-point number using the hardware sqrt register
    fix32_t fix32_Sqrt(fix32_t x);

    // usa: func_020c2c38
    int64_t GetHardwareDividerResult();
    // usa: func_020c2c5c
    // gets the hardware divider result right shifted 20 places.
    // use this in conjunction with fix32_QueueComputeReciprocal
    // or fix32_QueueComputeQuotient for async division of fix32 values.
    fix32_t fix32_GetDivisionResult();
    // usa: func_020c2c94
    // primes the hardware divider to compute 2^44 / n, where n is an integer.
    // thus for a fixed-point number x, the hardware divider will compute
    // the fixed point representation of 2^20 / x.
    // use in conjunction with fix32_GetDivisionResult to get the fixed point
    // representation of 1/x.
    void fix32_QueueComputeReciprocal(fix32_t x);

    // usa: func_020c2cc4
    // not used outside of this library
    // gets the hardware sqrt result right shifted 10 places.
    fix32_t fix32_GetSqrtResult();
    // usa: func_020c2cf0
    // primes the hardware divider to compute the integer (2^32*a) / b, where
    // a and b are integers. thus for fixed-point numbers x and y, the hardware
    // divider will compute the fixed-point representation of 2^20*(x/y).
    // use in conjunction with fix32_GetDivisionResult to get the fixed point
    // representation of x/y.
    void fix32_QueueComputeQuotient(fix32_t num, fix32_t denom);

    // usa: func_020c2d18
    // computes a/b using the hardware divider and returns the result.
    // only used in overlay 31
    int32_t FastIntDivide(int32_t a, int32_t b);
    // usa: func_020c2d54
    // computes a%b using the hardware divider and returns the result.
    // only used in overlay 31
    int32_t FastIntModulus(int32_t a, int32_t b);


    // usa: func_020c2d90
    void Vector3fix_Add(const Vector3fix* a, const Vector3fix* b, Vector3fix* out);
    // usa: func_020c2dc4
    void Vector3fix_Subtract(const Vector3fix* a, const Vector3fix* b, Vector3fix* out);
    // usa: func_020c2df8
    fix32_t Vector3fix_InnerProduct(const Vector3fix* a, const Vector3fix* b);
    // usa: func_020c2e34
    void Vector3fix_CrossProduct(const Vector3fix* a, const Vector3fix* b, Vector3fix* out);
    // usa: func_020c2eb8
    fix32_t Vector3fix_Length(const Vector3fix* vec);
    // usa: func_020c2f18
    void Vector3fix_Normalize(const Vector3fix* in, Vector3fix* out);
    // usa: func_020c3030
    // computes the length of the vector a-b
    fix32_t Vector3fix_Distance(const Vector3fix* a, const Vector3fix* b);

    // 0x020c30ac - 0x020c36ec: functions that compute stuff with a ton of
    // magic constants. Possibly software implementations of trig functions?
    // Will come back to this when the call sites come up

    // usa: func_020c338c
    fix32_t fix32_Atan2(fix32_t y, fix32_t x);
    // usa: func_020c3554
    // works like Atan2 but the range is rescaled so that pi = 0x8000,
    // so for example (x = 0, y > 0) gives 0x4000 = 4.0
    fix32_t fix32_Atan2_Rescaled(fix32_t y, fix32_t x);

#ifdef __cplusplus
}
#endif