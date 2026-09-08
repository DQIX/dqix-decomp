#pragma once

// The SDK provides a number of matrix operations and some basic vector
// stuff (add, subtract, normalize), but there are some additional ones earlier
// in the binary
#include "../System/Matrix.h"

fix32_t fix32sin(fix32_t x);
fix32_t fix32cos(fix32_t x);

// if a, b are angles in the range [-pi, pi], this computes b - a mod 2pi,
// taken to be in the range [-pi, pi]. Note pi = 0x3244 in fixed point
fix32_t fix32SignedAngleDistance(fix32_t a, fix32_t b);

// exactly the same as regular abs but ok
fix32_t fix32abs(fix32_t x);

Matrix4x3 RotationMatrixX(fix32_t angle);
Matrix4x3 RotationMatrixY(fix32_t angle);
Matrix4x3 RotationMatrixZ(fix32_t angle);

void Vector3fixMultiplyScalar(const Vector3fix* in, fix32_t scalar, Vector3fix* out);
// component-wise product
void Vector3fixMultiply(const Vector3fix* a, const Vector3fix* b, Vector3fix* out);

void Vector3fixDivideScalar(const Vector3fix* in, fix32_t scalar, Vector3fix* out);

fix32_t fix32ReduceAngle0To2Pi(fix32_t angle);

// maybe Vector3fix16::operator=? If so, it's explicitly defined
void Vector3fix16Copy(Vector3fix16* dst, const Vector3fix16* src);

fix32_t Vector3fixSquaredDistance(const Vector3fix* a, const Vector3fix* b);

// returns the atan2 angle of target - source in the (z, x)-plane, i.e. the
// angle is tan^{-1}(delta_x / delta_z). In particular, this is zero when the 
// vector from source to target points in the positive z-direction, pi/2 in
// the positive x-direction, pi (or -pi) in the negative z-direction and
// -pi/2 in the negative x-direction.
fix32_t Vector3fixAngleToPoint(const Vector3fix* source, const Vector3fix* target);