#ifndef MATH_H
#define MATH_H

typedef struct {
    double x;
    double y;
    double z;
} vec3_t;

typedef struct {
    double yaw;
    double pitch;
    double roll;
} euler_t;

typedef struct {
    vec3_t v[3];
} vec3x3_t;

double calculate_move_distance(double theta);
void vec_to_euler_angles(const vec3_t *v, euler_t *out);
void plane_normal(const vec3x3_t *plane, vec3_t *out);

#endif /* MATH_H */
