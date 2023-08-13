#pragma once

#include "shared_defines.h"

inline f32 Lerp(f32 a, f32 b, f32 t) {
    return a + (b - a) * t;
}

#define PI 3.14159265358979323846f
inline f32 RadToDeg(f32 rad) {
    return rad * (180.0f / PI);
}
inline f32 DegToRad(f32 deg) {
    return deg * (PI / 180.0f);
}

struct v2 {
    f32 x;
    f32 y;
};

// Operators
v2 operator+(v2 a, v2 b);
v2 operator-(v2 a, v2 b);
v2 operator*(v2 a, f32 b);
v2 operator/(v2 a, f32 b);
v2 operator*(f32 a, v2 b);
v2 operator/(f32 a, v2 b);

f32 Dot(v2 a, v2 b);
v2  Normalize(v2 v);
v2  Lerp(v2 a, v2 b, f32 t);
v2  Reflect(v2 v, v2 n);
f32 SignedAngle(v2 a, v2 b);
bool RoughlyEqual(v2 a, v2 b, f32 epsilon = 0.0001f);
bool RoughlyZero(v2 a, f32 epsilon = 0.0001f);

