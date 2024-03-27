
#ifndef MPU_H_
#define MPU_H_
// #include <stdio.h>
// #include <errno.h>
// #include <time.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "mpu_linear.h"

#define MPU6050_ADDR 0x68
#define PWR_MGMT_1   0x6b
#define SMPLRT_DIV   0x19
#define CONFIG       0x1a
#define GYRO_CONFIG  0x1b
#define ACCEL_CONFIG 0x1c
#define INT_PIN_CFG  0x37
#define INT_ENABLE   0x38
#define ACCEL_XOUT_H 0x3B
#define ACCEL_YOUT_H 0x3d
#define ACCEL_ZOUT_H 0x3f
#define TEMP_OUT_H   0x41
#define GYRO_XOUT_H  0x43
#define GYRO_YOUT_H  0x45
#define GYRO_ZOUT_H  0x47

// AK8963 registers
#define AK8963_ADDR   0x0c
#define AK8963_ST1    0x02
#define HXH          0x04
#define HYH          0x06
#define HZH          0x08
#define AK8963_ST2   0x09
#define AK8963_CNTL  0x0a
#define AK8963_ASAX  0x10
#define MAG_SENS 4800.f // magnetometer sensitivity: 4800 uT

#define ACCEL_RANGE_2G 0x00
#define ACCEL_RANGE_4G 0x08
#define ACCEL_RANGE_8G 0x10
#define ACCEL_RANGE_16G 0x18
#define GYRO_RANGE_250DPS 0x00
#define GYRO_RANGE_500DPS 0x08
#define GYRO_RANGE_1000DPS 0x10
#define GYRO_RANGE_2000DPS 0x18

#define MPU_ACCEL 0
#define MPU_GYRO 1

/*
struct vec3f {
    // float x, y, z;
    union {
        struct { float x, y, z; };
        float v[3];
    };
};
*/
enum fs_sel {
    FS_SEL_0,
    FS_SEL_1,
    FS_SEL_2,
    FS_SEL_3,
};

struct mpu_config {
    float accel_sens;
    float gyro_sens;
    float mag_sens;
    struct vec3f mag_coeff;
    float internal_sample_rate;
    float sample_rate;
};

struct mpu {
    int fd[2];
    struct mpu_config conf;
};

int
mpu_set_sample_rate_div(struct mpu *mpu, int const div);

int
mpu_get_sample_rate_div(struct mpu *mpu);

int
gyro_set_dlpf(struct mpu *mpu, int const val);

int
gyro_get_dlpf(struct mpu *mpu);

float
gyro_get_sample_rate(struct mpu *mpu);

int
gyro_set_sens(struct mpu *mpu, enum fs_sel sel);

int
accel_set_sens(struct mpu *mpu, enum fs_sel sel);

int
mpu_init(struct mpu *mpu);

struct vec3f
mpu_gyro(struct mpu *mpu);

struct vec3f
mpu_accel(struct mpu *mpu);

struct vec3f
mpu_mag(struct mpu *mpu);


#endif // MPU_H_
