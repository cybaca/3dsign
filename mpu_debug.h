#ifndef MPU_DEBUG_H_
#define MPU_DEBUG_H_

#include <errno.h>
#include <stdio.h>

#define PRINT_ERROR() \
    fprintf(stderr, \
        "%s:%d \"%s\" \"%s\" " \
        , __FILE__ \
        , __LINE__ \
        , __func__ \
        , strerror(errno) \
        )

#define LOG_ERROR(...) \
    fprintf(stderr, "Error: "), \
    fprintf(stderr, "%s\n", __VA_ARGS__), \
    fprintf(stderr, "%s:%d \"%s\" \"%s\" ", __FILE__, __LINE__, __func__, \
        strerror(errno)), \
    printf("\n")

#endif // MPU_DEBUG_H_
