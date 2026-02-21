#include <math.h>
#include <float.h>

#include "anglefix.h"
#include "mathh.h"

// thanks to not-a-zombie for most of this:
// https://github.com/not-a-zombie/vmf-resizer

double calculate_move_distance(double theta) {
    theta = fmodl(theta + 2.0 * M_PI, M_PI / 2.0);
    return (sqrtl(2.0) * cosl(theta - M_PI_4) - 1.0) * PLAYER_SIZE / 2.0;
}

void vec_to_euler_angles(const vec3_t *v, euler_t *out) {
    out->yaw = atan2l(v->y, v->x);
    out->pitch = atan2l(-v->z, sqrtl(v->x * v->x + v->y * v->y));
    out->roll = 0.0;
}

void plane_normal(const vec3x3_t *plane, vec3_t *out) {
    // u = v1 - v0
    double ux = plane->v[1].x - plane->v[0].x;
    double uy = plane->v[1].y - plane->v[0].y;
    double uz = plane->v[1].z - plane->v[0].z;

    // v = v2 - v0
    double vx = plane->v[2].x - plane->v[0].x;
    double vy = plane->v[2].y - plane->v[0].y;
    double vz = plane->v[2].z - plane->v[0].z;

    // Cross product u × v
    out->x = uy * vz - uz * vy;
    out->y = uz * vx - ux * vz;
    out->z = ux * vy - uy * vx;

    // Normalize
    double length = sqrtl(out->x * out->x + out->y * out->y + out->z * out->z);
    if (length == 0.0) {
        length = DBL_MIN;
    }

    out->x /= length;
    out->y /= length;
    out->z /= length;
}
