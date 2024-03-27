
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "mpu_debug.h"
#include "mpu_array.h"


static inline double
integrate(double *restrict v, double *restrict x, int const size)
{
    double f;
    int i;
    for (i = 0; i < size ; ++i)
        f += 0.5 * (x[i + 1] - x[i]) * (v[i] + v[i + 1]);
    return f;
}

/*
double *
cumtrapz(double *restrict f_x, double *restrict x, int const size)
{

    double *ret = malloc(sizeof(double) * (size + 1));
    int i;
    double const div = 1. / (double)N;
    for (i = 0; i <= size; ++i) {
        double x = (double)i * div;
        f_x[i] = sqrt(1. - pow(x[i], 2));
    }


}
*/

double *
cumsum(double *arr, int const size)
{
    if (size < 2) {
        fprintf(stderr, "Size must be at least 2\n");
        return NULL;
    }
    double *ret = malloc((size_t)size * sizeof(double));
    int i;
    ret[0] = arr[0];
    for (i = 1; i < size; ++i)
        ret[i] = ret[i - 1] + arr[i];
    return ret;
}

/*
double *
array_diff_f64(double *arr, int const size, int *newsize)
{
    if (size < 2) {
        fprintf(stderr, "Size must be at least 2\n");
        return NULL;
    }
    int new_size = size - 1;
    double *ret = malloc((size_t)new_size * sizeof(double));
    int i;
    ret[0] = arr[1] - arr[0];
    for (i = 0; i < new_size; ++i)
        ret[i] = arr[i + 1] - arr[i];

    *newsize = new_size;
    return ret;
}
*/

static inline void
array_diff_f64(double *restrict src, double *restrict dest, int const size)
{
    if (size < 2) {
        fprintf(stderr, "Size must be at least 2\n");
        return;
    }

    int new_size = size - 1;
    int i;
    for (i = 0; i < new_size; ++i)
        dest[i] = src[i + 1] - src[i];
}

double *
cumtrapz_f64(
    double *restrict f,
    double *restrict x,
    int const size,
    int *newsize
    )
{
    int new_size = size - 1;
    double *d = malloc((size_t)new_size * sizeof(double));
    double *ret = malloc((size_t)new_size * sizeof(double));
    double *arr = malloc((size_t)new_size * sizeof(double));
    memset(arr, 0, (size_t)new_size * sizeof(double));
    memset(ret, 0, (size_t)new_size * sizeof(double));

    array_diff_f64(x, d, size);
    int i;

    // for n_x in n_0...n_n, n_x + n_x+1
    for (i = 0; i < new_size; ++i) {
        arr[i] = f[i] + f[i + 1];
    }

    for (i = 0; i < new_size; ++i) {
        arr[i] *= d[i];
        arr[i] *= 0.5;
    }

    // cumulative sum
    ret[0] = arr[0];
    for (i = 1; i < new_size; ++i) {
        ret[i] = ret[i - 1] + arr[i];
    }

    free(arr);
    *newsize = new_size;
    return ret;
}


static inline void
array_diff_f32(float *restrict src, float *restrict dest, int const size)
{
    if (size < 2) {
        LOG_ERROR("size must be at least 2");
        return;
    }

    int new_size = size - 1;
    int i;
    for (i = 0; i < new_size; ++i)
        dest[i] = src[i + 1] - src[i];
}

float *
cumtrapz_f32(
    float *restrict f,
    float *restrict x,
    int const size,
    int *newsize
    )
{
    float *d = NULL; // time diffs
    float *ret = NULL; // cumulative integration values
    // float *arr = NULL;

    int new_size = size - 1;
    d = malloc((size_t)new_size * sizeof(float));
    ret = malloc((size_t)new_size * sizeof(float));
    // arr = malloc((size_t)new_size * sizeof(float));
    // memset(arr, 0, (size_t)new_size * sizeof(float));
    memset(ret, 0, (size_t)new_size * sizeof(float));

    // time diffs
    array_diff_f32(x, d, size);
    int i;

    float total = 0.f;
    for (i = 0; i < new_size; ++i) {
        float vel = (f[i] + f[i + 1]) * 0.5f;
        total += vel * d[i];
        ret[i] = total;
    }

    /*
    // for n_x in n_0...n_n, n_x + n_x+1
    for (i = 0; i < new_size; ++i) {
        arr[i] = (f[i] + f[i + 1]) * 0.5f;
    }

    for (i = 0; i < new_size; ++i) {
        arr[i] *= d[i];
    }

    // cumulative sum
    ret[0] = arr[0];
    for (i = 1; i < new_size; ++i) {
        ret[i] = ret[i - 1] + arr[i];
    }
    */

    // free(arr);
    free(d);
    *newsize = new_size;
    return ret;
}

float *
rot_displace(
    float *restrict w,
    float *restrict x,
    int size,
    int *restrict newsize)
{
    assert(newsize != NULL);
    // float samp_rate = (float)size / (x[size - 1] - x[0]);

    float *res = cumtrapz_f32(w, x, size, newsize);
    return res;
}

float
total_displacement(
    float *restrict w,
    float *restrict x,
    int size
    )
{
    float total = 0.f;
    int i;
    float *d = malloc((size_t)size * sizeof(float));

    array_diff_f32(x, d, size);

    for (i = 1; i < size; ++i)
        total += w[i] * d[i - 1];

    free(d);
    return total;
}

float
trapz_displacement(
    float *restrict w,
    float *restrict x,
    int size
    )
{
    float total = 0.f;
    int i;
    float *d = malloc((size_t)size * sizeof(float));

    array_diff_f32(x, d, size);

    for (i = 0; i < size - 1; ++i) {
        float vel = (w[i] + w[i + 1]) * 0.5f;
        total += vel * d[i];
    }

    free(d);
    return total;
}
