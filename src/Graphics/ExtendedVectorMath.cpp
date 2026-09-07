#include "Graphics/Vector.h"

#if defined(jpn)
#define data_020e9450 data_020e955c
#endif

extern const fix16_t data_020e9450[0x10000 * 2];

fix32_t fix32sin(fix32_t x)
{
    // The table has 0x10000 sine entries, and so using a 16-bit 
    // value ensures perfect wraparound for values outside of (0, 2pi)
    unsigned short normalizedAngle = (x << 16) / FIX_2PI;
    int tableIndex = normalizedAngle >> 4;

    return data_020e9450[tableIndex * 2];
}

fix32_t fix32cos(fix32_t x)
{
    // The table has 0x10000 cosine entries, and so using a 16-bit 
    // value ensures perfect wraparound for values outside of (0, 2pi)
    unsigned short normalizedAngle = (x << 16) / FIX_2PI;
    int tableIndex = normalizedAngle >> 4;

    return data_020e9450[tableIndex * 2 + 1];
}

fix32_t fix32SignedAngleDistance(fix32_t a, fix32_t b)
{
    fix32_t ret = 0;
    if (a < b)
    {
        ret = b - a;
        if (ret > 0x3244)
            ret -= 0x6488;
    }
    else if (b < a)
    {
        ret = -(a - b);
        if (ret < -0x3244)
            ret += 0x6488;
    }
    return ret;
}

fix32_t fix32abs(fix32_t x)
{
    if (x < 0)
        x = -x;
    return x;
}

Matrix4x3 RotationMatrixX(fix32_t angle)
{
    fix32_t sine = fix32sin(angle);
    fix32_t cosine = fix32cos(angle);

    Matrix4x3 ret;
    Mat4x3_WriteRotationX(&ret, sine, cosine);
    return ret;
}

Matrix4x3 RotationMatrixY(fix32_t angle)
{
    fix32_t sine = fix32sin(angle);
    fix32_t cosine = fix32cos(angle);

    Matrix4x3 ret;
    Mat4x3_WriteRotationY(&ret, sine, cosine);
    return ret;
}

Matrix4x3 RotationMatrixZ(fix32_t angle)
{
    fix32_t sine = fix32sin(angle);
    fix32_t cosine = fix32cos(angle);

    Matrix4x3 ret;
    Mat4x3_WriteRotationZ(&ret, sine, cosine);
    return ret;
}

void Vector3fixMultiplyScalar(const Vector3fix* in, fix32_t scalar, Vector3fix* out)
{
    out->x = FIX32_MULTIPLY(in->x, scalar);
    out->y = FIX32_MULTIPLY(in->y, scalar);
    out->z = FIX32_MULTIPLY(in->z, scalar);
}

void Vector3fixMultiply(const Vector3fix* a, const Vector3fix* b, Vector3fix* out)
{
    out->x = FIX32_MULTIPLY(a->x, b->x);
    out->y = FIX32_MULTIPLY(a->y, b->y);
    out->z = FIX32_MULTIPLY(a->z, b->z);
}

void Vector3fixDivideScalar(const Vector3fix* in, fix32_t scalar, Vector3fix* out)
{
    out->x = fix32_Divide(in->x, scalar);
    out->y = fix32_Divide(in->y, scalar);
    out->z = fix32_Divide(in->z, scalar);
}

fix32_t fix32ReduceAngle0To2Pi(fix32_t angle)
{
    if (angle < 0)
    {
        // bitwise AND gets floor of (-angle / 2pi)
        fix32_t periods = (fix32_Divide(-angle, FIX_2PI) + (1 << 12)) & ~0xfff;
        angle += FIX32_MULTIPLY(periods, FIX_2PI);
    }
    else if (angle > FIX_2PI)
    {
        fix32_t periods = fix32_Divide(angle, FIX_2PI) & ~0xfff;
        angle -= FIX32_MULTIPLY(periods, FIX_2PI);
    }

    return angle;
}

void Vector3fix16Copy(Vector3fix16* dst, const Vector3fix16* src)
{
    dst->x = src->x;
    dst->y = src->y;
    dst->z = src->z;
}

fix32_t Vector3fixSquaredDistance(const Vector3fix* a, const Vector3fix* b)
{
    fix32_t dx = a->x - b->x;
    fix32_t dy = a->y - b->y;
    fix32_t dz = a->z - b->z;

    fix32_t zsq = FIX32_MULTIPLY(dz, dz);
    fix32_t ysq = FIX32_MULTIPLY(dy, dy);
    fix32_t xsq = FIX32_MULTIPLY(dx, dx);

    fix32_t sum = xsq + ysq; // ...
    return xsq + ysq + zsq;
}

fix32_t Vector3fixAngleToPoint(const Vector3fix* source, const Vector3fix* target)
{
    Vector3fix difference;
    Vector3fix_Subtract(target, source, &difference);
    difference.y = 0;
    // not sure why we need this. is it just numerical stability?
    Vector3fix_Normalize(&difference, &difference);
    fix32_t angle = fix32_Atan2(difference.x, difference.z);
    return fix32ReduceAngle0To2Pi(angle);
}