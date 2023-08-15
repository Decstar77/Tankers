#include "shared_math.h"

#include <math.h>
#include <cmath>

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

bool CircleVsRect(Circle c, Rect r, CollisionManifold * manifold) {
    v2 closest = ClosestPointOnRect(c.pos, r);

    v2 diff = c.pos - closest;
    f32 distSq = Dot(diff, diff);
    f32 radiusSq = c.radius * c.radius;
    if (distSq > radiusSq) {
        return false;
    }

    if (manifold) {
        manifold->collided = true;
        manifold->normal = Normalize(diff);
        manifold->penetration = c.radius - sqrtf(distSq);
    }

    return true;
}

bool SweepCircleVsRect(Circle c, v2 c_vel, Rect r, v2 r_vel, SweepResult* result) {
    // Relative velocity
    v2 rel_vel = {c_vel.x - r_vel.x, c_vel.y - r_vel.y};

    // Expand the rectangle by the radius of the circle
    r.min.x -= c.radius;
    r.min.y -= c.radius;
    r.max.x += c.radius;
    r.max.y += c.radius;

    // Define an infinite time of impact and zero normal
    float tmin = 0.0f;
    float tmax = 1.0f;
    v2 normal = {0, 0};

    // Check each axis (x and y) for a collision
    for (int axis = 0; axis < 2; ++axis) {
        if (std::abs(rel_vel[axis]) < 1e-8) {
            // No relative movement along this axis, check if circle is outside the expanded rectangle
            if (c.pos[axis] < r.min[axis] || c.pos[axis] > r.max[axis]) {
                return false;
            }
        } else {
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
                normal = {0, 0};
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