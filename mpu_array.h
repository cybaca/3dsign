
#ifndef MPU_ARRAY_H_
#define MPU_ARRAY_H_
#include <string.h>
#include <stdlib.h>


struct array_f32 {
    size_t cap;
    size_t len;
    float *arr;
};

static inline void
array_f32_release(struct array_f32 *arr);

static inline void
array_f32_from_data(struct array_f32 *arr, float *data, size_t size);

static inline void
array_f32_init(struct array_f32 *arr);

static inline void
array_f32_grow(struct array_f32 *arr);

static inline void
array_f32_append(struct array_f32 *arr, float const value);

static inline size_t
array_f32_size(struct array_f32 *arr);

static inline float *
array_f32_data(struct array_f32 *arr);

static inline void
array_f32_clear(struct array_f32 *arr);

////////

static inline void
array_f32_clear(struct array_f32 *arr)
{
    // memset(arr->arr, 0, arr->cap * sizeof(float));
    arr->len = 0;
}

static inline void
array_f32_release(struct array_f32 *arr)
{
    if (arr->arr != NULL)
        free(arr->arr);

    memset(arr, 0, sizeof(*arr));
}

static inline void
array_f32_from_data(struct array_f32 *arr, float *data, size_t size)
{
    array_f32_release(arr);
    arr->cap = size;
    arr->len = size;
    arr->arr = malloc(sizeof(float) * size);
    memcpy(arr->arr, data, sizeof(float) * size);
}

static inline void
array_f32_init(struct array_f32 *arr)
{
    memset(arr, 0, sizeof(*arr));
    arr->cap = 32;
    arr->arr = malloc(sizeof(float) * arr->cap);
}

static inline void
array_f32_grow(struct array_f32 *arr)
{
    if (arr->cap <= arr->len + 1) {
        arr->cap *= 2;
        arr->arr = realloc(arr->arr, sizeof(float) * arr->cap);
    }
}

static inline void
array_f32_append(struct array_f32 *arr, float const value)
{
    array_f32_grow(arr);
    arr->arr[arr->len] = value;
    arr->len++;
}

static inline size_t
array_f32_size(struct array_f32 *arr)
{
    return arr->len;
}

static inline float *
array_f32_data(struct array_f32 *arr)
{
    return arr->arr;
}

#endif // MPU_ARRAY_H_
