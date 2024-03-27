
#ifndef MPU_LINEAR_H_
#define MPU_LINEAR_H_

#include <math.h>

// #define LIN_EPSILIN ((float)FLT_EPSILON)
#define LIN_EPSILON (0.0005f)
// #define RAD_CONST (M_PI / 180)
#define RAD_CONST (0.01745329f)
#define DEG_CONST (57.2957795f)

struct vec3f {
    union {
        struct { float x, y, z; };
        float v[3];
    };
};

static inline float
lin_to_radians(float const deg)
{
    return deg * RAD_CONST;
}

static inline float
lin_to_degrees(float const rad)
{
    return rad * DEG_CONST;
}

static inline int
lin_not_zero(float const x)
{
    return fabsf(x) > LIN_EPSILON;
}

static inline float
vec3f_length(struct vec3f const v)
{
    return sqrtf(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z);
}

static inline struct vec3f
vec3f_scale(struct vec3f const v, float const s)
{
    struct vec3f ret = {
        .x = v.x * s,
        .y = v.y * s,
        .z = v.z * s
    };
    return ret;
}

static inline struct vec3f
vec3f_normalized(struct vec3f const v)
{
    float const k = 1.f / vec3f_length(v);
    if (lin_not_zero(k))
        return vec3f_scale(v, k);
    return v;
}

static inline struct vec3f
vec3f_cross(struct vec3f const a, struct vec3f const b)
{
    struct vec3f ret = {
        .x = a.y * b.z - a.z * b.y,
        .y = a.z * b.x - a.x * b.z,
        .z = a.x * b.y - a.y * b.x
        };
    return ret;
}


#endif // MPU_LINEAR_H_
