#pragma once

#include "shared_defines.h"

inline f32 Lerp(f32 a, f32 b, f32 t) {
    return a + (b - a) * t;
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

v2 Normalize(v2 v);
v2 Lerp(v2 a, v2 b, f32 t);



