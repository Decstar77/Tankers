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

inline f32 Min(f32 a, f32 b) {
    return a < b ? a : b;
}

inline f32 Max(f32 a, f32 b) {
    return a > b ? a : b;
}

inline f32 Clamp(f32 a, f32 min, f32 max) {
    return a < min ? min : (a > max ? max : a);
}

inline i32 Clamp(i32 a, i32 min, i32 max) {
    return a < min ? min : (a > max ? max : a);
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

v2 V2(f32 x, f32 y);
v2 V2(i32 x, i32 y);

// Operators
v2 operator+(v2 a, v2 b);
v2 operator-(v2 a, v2 b);
v2 operator*(v2 a, f32 b);
v2 operator*(v2 a, v2 b);
v2 operator/(v2 a, f32 b);
v2 operator*(f32 a, v2 b);
v2 operator/(f32 a, v2 b);

v2      Min(v2 a, v2 b);
v2      Max(v2 a, v2 b);
f32     Dot(v2 a, v2 b);
v2      Normalize(v2 v);
v2      Lerp(v2 a, v2 b, f32 t);
v2      Reflect(v2 v, v2 n);
v2      Clamp(v2 v, v2 min, v2 max);
f32     SignedAngle(v2 a, v2 b);
bool    RoughlyEqual(v2 a, v2 b, f32 epsilon = 0.0001f);
bool    RoughlyZero(v2 a, f32 epsilon = 0.0001f);

struct Circle {
    v2 pos;
    f32 radius;
};

struct Rect {
    v2 min;
    v2 max;
};

enum BoundsType {
    BOUNDS_TYPE_INVALID = 0,
    BOUNDS_TYPE_CIRCLE,
    BOUNDS_TYPE_RECT,
};

struct Bounds {
    BoundsType type;
    union {
        Circle  circle;
        Rect    rect;
    };
};

struct CollisionManifold {
    v2 normal;
    f32 penetration;
};

bool    IsPointInCircle(v2 point, Circle circle);
bool    IsPointInRect(v2 point, Rect rect);
v2      ClosestPointOnCircle(v2 point, Circle circle);
v2      ClosestPointOnRect(v2 point, Rect rect);
bool    CircleVsCircle(Circle c1, Circle c2, CollisionManifold * manifold = nullptr);
bool    CircleVsRect(Circle c, Rect r, CollisionManifold * manifold = nullptr);

struct SweepResult {
    f32 t;
    v2 normal;
};

bool SweepCircleVsCircle(Circle c1, v2 c1_vel, Circle c2, v2 c2_vel, SweepResult * result);
bool SweepCircleVsRect(Circle c, v2 c_vel, Rect r, v2 r_vel, SweepResult * result);

struct fp {
    i32 value;
};

fp Fp(i32 value);
fp Fp(f32 value);

f32 FpToFloat(fp a);
i32 FpToInt(fp a);
i64 I64(fp f);

fp   Mod(fp a, fp b);
fp   Sin(fp v);
fp   Cos(fp v);

fp operator+(fp a, fp b);
fp operator-(fp a, fp b);
fp operator*(fp a, fp b);
fp operator/(fp a, fp b);

bool operator==(fp a, fp b);
bool operator!=(fp a, fp b);
bool operator>=(fp a, fp b);
bool operator<=(fp a, fp b);
bool operator>(fp a, fp b);
bool operator<(fp a, fp b);

fp & operator+=(fp & a, fp b);
fp & operator-=(fp & a, fp b);
fp & operator*=(fp & a, fp b);
fp & operator/=(fp & a, fp b);

const fp FP_PI = Fp(3.14159265358979323846f);
const fp FP_PI2 = Fp(6.28318530717958647692f);
const fp FP_HALF_PI = Fp(1.57079632679489661923f);

struct v2fp {
    fp x;
    fp y;

    fp & operator[](int i) {
        return (&x)[i];
    }
};

v2fp V2fp(fp x, fp y);
v2fp V2fp(f32 x, f32 y);
v2fp V2fp(i32 x, i32 y);
v2fp V2fp(v2 v);

v2   V2(v2fp v);

v2fp Min(v2fp a, v2fp b);
v2fp Max(v2fp a, v2fp b);
fp   Dot(v2fp a, v2fp b);
v2fp Normalize(v2fp v);
v2fp Lerp(v2fp a, v2fp b, fp t);
v2fp Reflect(v2fp v, v2fp n);
v2fp Clamp(v2fp v, v2fp min, v2fp max);
fp   SignedAngle(v2fp a, v2fp b);
bool RoughlyEqual(v2fp a, v2fp b, fp epsilon = Fp(0.0001f));
bool RoughlyZero(v2fp a, fp epsilon = Fp(0.0001f));

v2fp operator+(v2fp a, v2fp b);
v2fp operator-(v2fp a, v2fp b);
v2fp operator*(v2fp a, fp b);
v2fp operator/(v2fp a, fp b);

v2fp operator*(fp a, v2fp b);
v2fp operator/(fp a, v2fp b);



