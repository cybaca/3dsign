#ifndef MPU_TIME_H_
#define MPU_TIME_H_
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#define NANOSECOND 1000000000

static double const NANO_DIV = 1. / 1000000000.;

static inline int
mpu_sleep(double const seconds)
{
    int64_t nanoseconds = (int64_t)(seconds * (double)NANOSECOND);
    struct timespec in;
    struct timespec left;

    in.tv_sec = 0;
    in.tv_nsec = 0;

    while (nanoseconds >= NANOSECOND) {
        in.tv_sec += 1;
        nanoseconds -= NANOSECOND;
    }

    in.tv_nsec = nanoseconds;

    return nanosleep(&in, &left);
}

static inline uint64_t
mpu_nanoseconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * (uint64_t)NANOSECOND + (uint64_t)ts.tv_nsec;
}

static inline uint64_t
mpu_milliseconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * (uint64_t)1000 +
        (uint64_t)tv.tv_usec / (uint64_t)1000;
}

static inline float
mpu_seconds(void)
{
    uint64_t nano = mpu_nanoseconds();
    float seconds = (float)((double)nano * NANO_DIV);
    return seconds;
}


#endif // MPU_TIME_H_
