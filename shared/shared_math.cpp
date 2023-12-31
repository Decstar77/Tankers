#include "shared_math.h"

#include <math.h>
#include <cmath>

v2 V2(f32 x, f32 y) {
    v2 v = {};
    v.x = x;
    v.y = y;
    return v;
}

v2 V2(i32 x, i32 y) {
    v2 v = {};
    v.x = (f32)x;
    v.y = (f32)y;
    return v;
}

v2 operator+(v2 a, v2 b) {
    return { a.x + b.x, a.y + b.y };
}

v2 operator-(v2 a, v2 b) {
    return { a.x - b.x, a.y - b.y };
}

v2 operator*(v2 a, f32 b) {
    return { a.x * b, a.y * b };
}

v2 operator*(v2 a, v2 b) {
    return { a.x * b.x, a.y * b.y };
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

v2 Min(v2 a, v2 b) {
    return { Min(a.x, b.x), Min(a.y, b.y) };
}

v2 Max(v2 a, v2 b) {
    return { Max(a.x, b.x), Max(a.y, b.y) };
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

v2 Clamp(v2 v, v2 min, v2 max) {
    return { Clamp(v.x, min.x, max.x), Clamp(v.y, min.y, max.y) };
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

bool IsPointInCircle(v2 point, Circle circle) {
    v2 diff = point - circle.pos;
    f32 distSq = Dot(diff, diff);
    return distSq < circle.radius * circle.radius;
}

bool IsPointInRect(v2 point, Rect rect) {
    return point.x >= rect.min.x && point.x <= rect.max.x && point.y >= rect.min.y && point.y <= rect.max.y;
}

v2 ClosestPointOnCircle(v2 point, Circle circle) {
    v2 diff = point - circle.pos;
    f32 dist = sqrtf(Dot(diff, diff));
    if (dist < circle.radius) {
        return point;
    }
    else {
        return circle.pos + diff / dist * circle.radius;
    }
}

v2 ClosestPointOnRect(v2 point, Rect rect) {
    if (point.x < rect.min.x) {
        point.x = rect.min.x;
    }
    else if (point.x > rect.max.x) {
        point.x = rect.max.x;
    }
    else {
        point.x = point.x;
    }
    if (point.y < rect.min.y) {
        point.y = rect.min.y;
    }
    else if (point.y > rect.max.y) {
        point.y = rect.max.y;
    }
    else {
        point.y = point.y;
    }

    return point;
}

bool CircleVsCircle(Circle c1, Circle c2, CollisionManifold * manifold) {
    f32 radiusSum = c1.radius + c2.radius;
    v2 diff = c1.pos - c2.pos;
    f32 distSq = Dot(diff, diff);
    if (distSq > radiusSum * radiusSum) {
        return false;
    }

    if (manifold) {
        f32 dist = sqrtf(distSq);
        manifold->penetration = radiusSum - dist;
        manifold->normal = diff / dist;
    }

    return true;
}

bool CircleVsRect(Circle c, Rect r, CollisionManifold * manifold) {
    v2 closest = ClosestPointOnRect(c.pos, r);

    v2 diff = c.pos - closest;
    f32 distSq = Dot(diff, diff);
    f32 radiusSq = c.radius * c.radius;
    if (distSq > radiusSq) {
        return false;
    }

    if (manifold) {
        manifold->normal = Normalize(diff);
        manifold->penetration = c.radius - sqrtf(distSq);
    }

    return true;
}

bool IsPointInBounds(v2 point, Bounds bounds) {
    switch (bounds.type) {
    case BOUNDS_TYPE_CIRCLE:
        return IsPointInCircle(point, bounds.circle);
    case BOUNDS_TYPE_RECT:
        return IsPointInRect(point, bounds.rect);
    default:
        return false;
    }
}

v2 ClosestPointOnBounds(v2 point, Bounds bounds) {
    switch (bounds.type) {
    case BOUNDS_TYPE_CIRCLE:
        return ClosestPointOnCircle(point, bounds.circle);
    case BOUNDS_TYPE_RECT:
        return ClosestPointOnRect(point, bounds.rect);
    default:
        return point;
    }
}

#include <stdio.h>
bool SweepCircleVsCircle(Circle c1, v2 c1_vel, Circle c2, v2 c2_vel, SweepResult * result) {
    // Relative velocity of circle1 with respect to circle2
    v2 rel_vel = { c1_vel.x - c2_vel.x, c1_vel.y - c2_vel.y };

    if (RoughlyZero(rel_vel)) {
        CollisionManifold manifold = {};
        bool res = CircleVsCircle(c1, c2, &manifold);
        if (res && result) {
            result->t = 0.0f;
            result->normal = manifold.normal;
        }

        return res;
    }

    // Relative position of circle1 with respect to circle2
    v2 rel_pos = { c1.pos.x - c2.pos.x, c1.pos.y - c2.pos.y };

    // Combined radius of the two circles
    float combined_radius = c1.radius + c2.radius;

    // Quadratic equation coefficients
    float a = rel_vel.x * rel_vel.x + rel_vel.y * rel_vel.y;
    float b = 2.0f * (rel_pos.x * rel_vel.x + rel_pos.y * rel_vel.y);
    float c = rel_pos.x * rel_pos.x + rel_pos.y * rel_pos.y - combined_radius * combined_radius;

    // Discriminant
    float discriminant = b * b - 4.0f * a * c;

    // Check for no collision
    if (discriminant < 0.0f) {
        return false;
    }

    // Solve the quadratic equation
    float sqrt_discriminant = sqrtf(discriminant);
    float t1 = (-b - sqrt_discriminant) / (2.0f * a);
    float t2 = (-b + sqrt_discriminant) / (2.0f * a);

    // Check if both solutions are outside the range [0, 1]
    if (t1 > 1.0f || t2 < 0.0f) {
        return false;
    }

    // Take the earliest time within the range [0, 1]
    float t = Max(0.0f, t1);

    // Compute the collision normal
    v2 normal = { rel_pos.x + t * rel_vel.x, rel_pos.y + t * rel_vel.y };
    float length = sqrtf(normal.x * normal.x + normal.y * normal.y);
    normal.x /= length;
    normal.y /= length;

    // If there is a result pointer, set the time and normal
    if (result) {
        result->t = t;
        result->normal = normal;
    }

    return true;
}

bool SweepCircleVsRect(Circle c, v2 c_vel, Rect r, v2 r_vel, SweepResult * result) {
    // Relative velocity
    v2 rel_vel = { c_vel.x - r_vel.x, c_vel.y - r_vel.y };

    // Expand the rectangle by the radius of the circle
    r.min.x -= c.radius;
    r.min.y -= c.radius;
    r.max.x += c.radius;
    r.max.y += c.radius;

    // Define an infinite time of impact and zero normal
    float tmin = 0.0f;
    float tmax = 1.0f;
    v2 normal = { 0, 0 };

    // Check each axis (x and y) for a collision
    for (int axis = 0; axis < 2; ++axis) {
        if (std::abs(rel_vel[axis]) < 1e-8) {
            // No relative movement along this axis, check if circle is outside the expanded rectangle
            if (c.pos[axis] < r.min[axis] || c.pos[axis] > r.max[axis]) {
                return false;
            }
        }
        else {
            // Compute time of impact for both sides of the rectangle
            float t0 = (r.min[axis] - c.pos[axis]) / rel_vel[axis];
            float t1 = (r.max[axis] - c.pos[axis]) / rel_vel[axis];

            // Swap times if needed
            if (t0 > t1) {
                Swap(&t0, &t1);
            }

            // Update the normal if needed
            if (t0 > tmin) {
                tmin = t0;
                normal = { 0, 0 };
                normal[axis] = rel_vel[axis] < 0 ? 1.0f : -1.0f;
            }

            // Update the maximum time of impact
            tmax = Min(tmax, t1);

            // If the times become disjoint, there's no collision
            if (tmin > tmax) {
                return false;
            }
        }
    }

    // If there is a result pointer, set the time and normal
    if (result) {
        result->t = tmin;
        result->normal = normal;
    }

    return true;
}

fp Fp(i32 value) {
    fp p = {};
    p.value = value << 16;
    return p;
}

fp Fp(f32 f) {
    fp p = {};
    p.value = i32(f * f32(1 << 16) + (f >= 0 ? 0.5 : 0.5));
    return p;
}

f32 FpToFloat(fp a) {
    return f32(a.value) / f32(1 << 16);
}

i32 FpToInt(fp a) {
    return a.value >> 16;
}

i64 I64(fp f) {
    return (i64)FpToInt(f);
}

fp Mod(fp a, fp b) {
    fp v = {};
    v.value = a.value % b.value;
    return v;
}

fp Sin(fp x) {
    // This sine uses a fifth-order curve-fitting approximation originally
    // described by Jasper on coranac.com which has a worst-case
    // relative error of 0.07% (over [-pi:pi]).

    // Turn x from [0..2*PI] domain into [0..4] domain
    x = Mod(x, FP_PI2);
    x = x / FP_HALF_PI;

    // Take x modulo one rotation, so [-4..+4].
    if (x < Fp(0)) {
        x = x + Fp(4);
    }

    int sign = +1;
    if (x > Fp(2)) {
        // Reduce domain to [0..2].
        sign = -1;
        x = x - Fp(2);
    }

    if (x > Fp(1)) {
        // Reduce domain to [0..1].
        x = Fp(2) - x;
    }

    const fp x2 = x * x;

    return Fp(sign) * x * (FP_PI - x2 * (FP_PI2 - Fp(5) - x2 * (FP_PI - Fp(3)))) / Fp(2);
}

fp Cos(fp v) {
    return Sin(v + FP_HALF_PI);
}

fp operator+(fp a, fp b) {
    fp p = {};
    p.value = a.value + b.value;
    return p;
}

fp operator-(fp a, fp b) {
    fp p = {};
    p.value = a.value - b.value;
    return p;
}

fp operator*(fp a, fp b) {
    fp p = {};
    p.value = i32((i64(a.value) * i64(b.value)) >> 16);
    return p;
}

fp operator/(fp a, fp b) {
    fp p = {};
    p.value = i32((i64(a.value) << 16) / i64(b.value));
    return p;
}

bool operator==(fp a, fp b) {
    return a.value == b.value;
}

bool operator!=(fp a, fp b) {
    return a.value != b.value;
}

bool operator>=(fp a, fp b) {
    return a.value >= b.value;
}

bool operator<=(fp a, fp b) {
    return a.value <= b.value;
}

bool operator>(fp a, fp b) {
    return a.value > b.value;
}

bool operator<(fp a, fp b) {
    return a.value < b.value;
}

fp & operator+=(fp & a, fp b) {
    a.value += b.value;
    return a;
}

fp & operator-=(fp & a, fp b) {
    a.value -= b.value;
    return a;
}

fp & operator*=(fp & a, fp b) {
    a.value = i32((i64(a.value) * i64(b.value)) >> 16);
    return a;
}

fp & operator/=(fp & a, fp b) {
    a.value = i32((i64(a.value) << 16) / i64(b.value));
    return a;
}

v2fp V2fp(fp x, fp y) {
    return { x, y };
}

v2fp V2fp(f32 x, f32 y) {
    return { Fp(x), Fp(y) };
}

v2fp V2fp(i32 x, i32 y) {
    return { Fp(x), Fp(y) };
}

v2fp V2fp(v2 v) {
    return { Fp(v.x), Fp(v.y) };
}

v2 V2(v2fp v) {
    return { FpToFloat(v.x), FpToFloat(v.y) };
}

fp Dot(v2fp a, v2fp b) {
    return a.x * b.x + a.y * b.y;
}

fp DistanceSqrd(v2fp a, v2fp b) {
    v2fp diff = a - b;
    return Dot(diff, diff);
}

v2fp operator+(v2fp a, v2fp b) {
    return { a.x + b.x, a.y + b.y };
}

v2fp operator-(v2fp a, v2fp b) {
    return { a.x - b.x, a.y - b.y };
}

v2fp operator*(v2fp a, fp b) {
    return { a.x * b, a.y * b };
}

v2fp operator*(v2fp a, v2fp b) {
    return { a.x * b.x, a.y * b.y };
}

v2fp operator/(v2fp a, fp b) {
    return { a.x / b, a.y / b };
}

v2fp operator*(fp a, v2fp b) {
    return { a * b.x, a * b.y };
}

v2fp operator/(fp a, v2fp b) {
    return { a / b.x, a / b.y };
}

v2fp GetBoundsFpCenter(Boundsfp bounds) {
    switch (bounds.type) {
    case BOUNDS_TYPE_CIRCLE:
        return bounds.circle.pos;
    case BOUNDS_TYPE_RECT:
        return (bounds.rect.min + bounds.rect.max) / Fp(2);
    default:
        return {};
    }
}

