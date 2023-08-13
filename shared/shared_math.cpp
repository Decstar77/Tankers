#include "shared_math.h"

#include <math.h>

v2 operator+(v2 a, v2 b) {
    return { a.x + b.x, a.y + b.y };
}

v2 operator-(v2 a, v2 b) {
    return { a.x - b.x, a.y - b.y };
}

v2 operator*(v2 a, f32 b) {
    return { a.x * b, a.y * b };
}

v2 operator/(v2 a, f32 b) {
    return { a.x / b, a.y / b };
}

v2 operator*(f32 a, v2 b) {
    return { a * b.x, a * b.y };
}

v2 operator/(f32 a, v2 b) {
    return { a / b.x, a / b.y };
}

f32 Dot(v2 a, v2 b) {
    return a.x * b.x + a.y * b.y;
}

v2 Normalize(v2 v) {
    return v / sqrtf(v.x * v.x + v.y * v.y);
}

v2 Lerp(v2 a, v2 b, f32 t) {
    return a + (b - a) * t;
}

v2 Reflect(v2 v, v2 n) {
    return v - 2.0f * Dot(v, n) * n;
}

f32 SignedAngle(v2 a, v2 b) {
    f32 angle = atan2f(b.y, b.x) - atan2f(a.y, a.x);
    if (angle < -PI) {
        angle += 2.0f * PI;
    }
    else if (angle > PI) {
        angle -= 2.0f * PI;
    }
    return angle;
}

bool RoughlyEqual(v2 a, v2 b, f32 epsilon) {
    return fabsf(a.x - b.x) < epsilon && fabsf(a.y - b.y) < epsilon;
}

bool RoughlyZero(v2 a, f32 epsilon) {
    return fabsf(a.x) < epsilon && fabsf(a.y) < epsilon;
}


