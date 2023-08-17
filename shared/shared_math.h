#pragma once

#include "shared_defines.h"

#define PI 3.14159265358979323846f

inline f32 RadToDeg(f32 rad) {
    return rad * (180.0f / PI);
}

inline f32 DegToRad(f32 deg) {
    return deg * (PI / 180.0f);
}

inline f32 Lerp(f32 a, f32 b, f32 t) {
    return a + (b - a) * t;
}

inline f32 Min(f32 a, f32 b){
    return a < b ? a : b;
}

inline f32 Max(f32 a, f32 b){
    return a > b ? a : b;
}

inline void Swap(f32 * a, f32 * b) {
    f32 temp = *a;
    *a = *b;
    *b = temp;
}

inline f32 LerpAngle(f32 a, f32 b, f32 t) {
    f32 angle = b - a;
    if (angle < -PI) {
        angle += 2.0f * PI;
    }
    else if (angle > PI) {
        angle -= 2.0f * PI;
    }
    return a + angle * t;
}

struct v2 {
    f32 x;
    f32 y;

    f32 & operator[](int i) {
        return (&x)[i];
    }
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

struct Circle {
    v2 pos;
    f32 radius;
};

struct Rect {
    v2 min;
    v2 max;
};

struct CollisionManifold {
    v2 normal;
    f32 penetration;
};

v2 ClosestPointOnCircle(v2 point, Circle circle);
v2 ClosestPointOnRect(v2 point, Rect rect);
bool CircleVsCircle(Circle c1, Circle c2, CollisionManifold * manifold = nullptr);
bool CircleVsRect(Circle c, Rect r, CollisionManifold * manifold = nullptr);

struct SweepResult {
    f32 t;
    v2 normal;
};

bool SweepCircleVsCircle(Circle c1, v2 c1_vel, Circle c2, v2 c2_vel, SweepResult* result);
bool SweepCircleVsRect(Circle c, v2 c_vel, Rect r, v2 r_vel, SweepResult* result);

