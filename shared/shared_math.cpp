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

v2 Normalize(v2 v) {
    return v / sqrtf(v.x * v.x + v.y * v.y);
}

v2 Lerp(v2 a, v2 b, f32 t){
    return a + (b - a) * t;
}
