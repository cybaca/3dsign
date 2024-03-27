#ifndef MPU_INTEGRAL_H_
#define MPU_INTEGRAL_H_


float *
cumtrapz_f32(
    float *restrict f,
    float *restrict x,
    int const size,
    int *newsize
    );

float *
rot_displace(
    float *restrict w,
    float *restrict x,
    int size,
    int *restrict newsize
    );

float
total_displacement(
    float *restrict w,
    float *restrict x,
    int size
    );

float
trapz_displacement(
    float *restrict w,
    float *restrict x,
    int size
    );
#endif // MPU_INTEGRAL_H_
