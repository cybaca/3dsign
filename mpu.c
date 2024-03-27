
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "mpu_debug.h"
#include "mpu_time.h"
#include "mpu_i2c.h"
#include "mpu.h"


static float const MPU_DIV = powf(2.f, 15.f);

static int const CONFIG_SEL[] = {
    0b00000, // 0
    0b01000, // 8
    0b10000, // 16
    0b11000  // 24
};

/*
static inline int
_get_fs_sel(enum fs_sel sel)
{
    return CONFIG_SEL[sel];
}
*/


static int
_ak8963_init(struct mpu *mpu);

static int
_mpu6050_init(struct mpu *mpu);

static inline int
_ak8963_read(struct mpu *mpu, int const reg);

static inline float
_mag_convert(int const mag);

static inline int
_read_raw_bits(struct mpu *mpu, int const reg);

static int
_mpu6050_reset_power_settings(struct mpu *mpu);

static inline int
_ak8963_read(struct mpu *mpu, int const reg)
{
    //  read magnetometer values
    int low = wiringPiI2CReadReg8(mpu->fd[1], reg - 1);
    if (low < 0) {
        LOG_ERROR("wiringPiI2CReadReg8");
        return INT_MAX;
    }
    int high = wiringPiI2CReadReg8(mpu->fd[1], reg);
    if (high < 0) {
        LOG_ERROR("wiringPiI2CReadReg8");
        return INT_MAX;
    }

    // combine high and low for unsigned bit value
    int value = ((high << 8) | low);

    // convert to +- value
    if (value > 32768)
        value -= 65536;
    return value;
}

static inline float
_mag_convert(int const mag)
{
    return ((float)mag / MPU_DIV) * MAG_SENS;
}

static inline int
_read_raw_bits(struct mpu *mpu, int const reg)
{
    // read accel and gyro values
    int high = wiringPiI2CReadReg8(mpu->fd[0], reg);
    if (high < 0) {
        LOG_ERROR("wiringPiI2CReadReg8");
        return INT_MAX;
    }
    int low = wiringPiI2CReadReg8(mpu->fd[0], reg + 1);
    if (low < 0) {
        LOG_ERROR("wiringPiI2CReadReg8");
        return INT_MAX;
    }

    // combine higha and low for unsigned bit value
    int value = ((high << 8) | low);

    // convert to +- value
    if (value > 32768)
        value -= 65536;
    return value;
}

/*
int
_ak8963_release(struct mpu *mpu)
{
    wiringPiI2CClose(mpu->fd[1]);
    return 0;
}

int
_mpu6050_release(struct mpu *mpu)
{
    wiringPiI2CClose(mpu->fd[0]);
    return 0;
}
*/

static int
_mpu6050_reset_power_settings(struct mpu *mpu)
{
    int result = 0;
    int fd = mpu->fd[0];
    // Reset the internal registers and restores the default settings.
    // Write a 1 to set the reset, the bit will auto clear.
    result = wiringPiI2CWriteReg8(fd, PWR_MGMT_1, 0x80 & 0xff);
    if (result < 0) {
        LOG_ERROR("wiringPiI2CWriteReg8");
        return result;
    }
    mpu_sleep(0.1);

    // Clear internal registers just in case
    result = wiringPiI2CWriteReg8(fd, PWR_MGMT_1, 0x00);
    if (result < 0) {
        LOG_ERROR("wiringPiI2CWriteReg8d");
        return result;
    }
    mpu_sleep(0.1);

    // CLKSEL 1 Auto select the best clock source, else internal oscillator
    result = wiringPiI2CWriteReg8(fd, PWR_MGMT_1, 0x01);
    if (result < 0) {
        LOG_ERROR("wiringPiI2CWriteReg8");
        return result;
    }
    mpu_sleep(0.1);

    return result;
}

static int
_mpu6050_init(struct mpu *mpu)
{
    // reset all sensors
    int result = 0;
    int fd = -1;

    fd = wiringPiI2CSetup(MPU6050_ADDR);

    mpu->fd[0] = fd;

    result = _mpu6050_reset_power_settings(mpu);
    if (result < 0) {
        LOG_ERROR("_mpu6050_reset_power_settings(mpu)");
        return result;
    }

    result = mpu_set_sample_rate_div(mpu, 0);
    if (result < 0) {
        LOG_ERROR("mpu_set_sample_rate_div(fd, 0)");
        return result;
    }

    // Set configuration register to all 0s
    result = wiringPiI2CWriteReg8(fd, CONFIG, 0);
    if (result < 0) {
        LOG_ERROR("wiringPiI2CWriteReg8");
        return result;
    }
    mpu_sleep(0.1);

    if ((result = gyro_set_sens(mpu, FS_SEL_0)) < 0) {
        LOG_ERROR("gyro_set_sens(mpu, FS_SEL_0)");
        return result;
    }


    if ((result = accel_set_sens(mpu, FS_SEL_0)) < 0) {
        LOG_ERROR("accel_set_sens(mpu, FS_SEL_0)");
        return result;
    }

    // set interupt bypass enable bit
    // set latch int enable bit
    // interrupt register (related to overflow of data [FIFO])
    result = wiringPiI2CWriteReg8(fd, INT_PIN_CFG, 0x22);
    if (result < 0) {
        LOG_ERROR("wiringPiI2CWriteReg8");
        return result;
    }
    mpu_sleep(0.1);

    // enable the ak8963 magnetometer in pass-through mode
    result = wiringPiI2CWriteReg8(fd, INT_ENABLE, 1 & 0xff);
    if (result < 0) {
        LOG_ERROR("wiringPiI2CWriteReg8");
        return result;
    }
    mpu_sleep(0.1);

    mpu->fd[0] = fd;
    return 0;
}

static int
_ak8963_init(struct mpu *mpu)
{
    int fd = -1;
    int result = 0;
    char coeff_data[3] = { 0 };

    fd = wiringPiI2CSetup(AK8963_ADDR);
    result = wiringPiI2CWriteReg8(fd, AK8963_CNTL, 0x00);
    if (result < 0) {
        LOG_ERROR("wiringPiI2CWriteReg8(fd, AK8963_CNTL, 0x00)");
        return result;
    }
    mpu_sleep(0.1);

    result = wiringPiI2CWriteReg8(fd, AK8963_CNTL, 0x0f);
    if (result < 0) {
        LOG_ERROR("wiringPiI2CWriteReg8(fd, AK8963_CNTL, 0x0f)");
        return result;
    }

    mpu_sleep(0.1);

    // ssize_t res = 0;
    // res = read(fd, coeff_data, 3);

    // coeff_data = bus.read_i2c_block_data(AK8963_ADDR, AK8963_ASAX, 3)
    result = i2c_read_block(fd, AK8963_ASAX, 3, coeff_data);
    if (result < 0) {
        LOG_ERROR("i2c_read_block(fd, AK8963_ASAX, 3, coeff_data)");
        return result;
    }

    
    struct vec3f coeff;
    coeff.x = 0.5f * (float)(coeff_data[0] - 128) / 256.f + 1.f;
    coeff.y = 0.5f * (float)(coeff_data[1] - 128) / 256.f + 1.f;
    coeff.z = 0.5f * (float)(coeff_data[2] - 128) / 256.f + 1.f;

    // AK8963_coeffx = (0.5 * (coeff_data[0] - 128)) / 256.0 + 1.0
    // AK8963_coeffy = (0.5 * (coeff_data[1] - 128)) / 256.0 + 1.0
    // AK8963_coeffz = (0.5 * (coeff_data[2] - 128)) / 256.0 + 1.0
    mpu_sleep(0.1);

    result = wiringPiI2CWriteReg8(fd, AK8963_CNTL, 0x00);
    if (result < 0) {
        LOG_ERROR("wiringPiI2CWriteReg8(fd, AK8963_CNTL, 0x00)");
        return result;
    }
    mpu_sleep(0.1);

    int AK8963_bit_res = 0b0001; // 0b0001 = 16-bit
    int AK8963_samp_rate = 0b0110; // 0b0010 = 8 Hz, 0b0110 = 100 Hz
    int AK8963_mode = (AK8963_bit_res << 4) + AK8963_samp_rate; //bit conversion
    result = wiringPiI2CWriteReg8(fd, AK8963_CNTL, AK8963_mode);
    if (result < 0) {
        LOG_ERROR(
            "wiringPiI2CWriteReg8(fd, AK8963_CNTL, AK8963_mode)");
        return result;
    }
    mpu_sleep(0.1);

    mpu->fd[1] = fd;
    mpu->conf.mag_sens = MAG_SENS;
    memcpy(&mpu->conf.mag_coeff, &coeff, sizeof(mpu->conf.mag_coeff));

    // return [AK8963_coeffx, AK8963_coeffy, AK8963_coeffz]
    return 0;
}

/**
 * SMPLRT_DIV register 
 * Divides the internal sample rate (see register CONFIG) to generate the
 * sample rate that controls sensor data output rate, FIFO sample rate.
 * NOTE: This register is only effective when Fchoice = 2’b11 (fchoice_b
 * register bits are 2’b00), and (0 < dlpf_cfg < 7), such that the average
 * filter’s output is selected (see chart below).
*/
// sample rate = 8kHz(internal sample rate) / (1 + div)
// alter sample rate (stability)
int
mpu_set_sample_rate_div(struct mpu *mpu, int const div)
{
    int result = 0;
    result = wiringPiI2CWriteReg8(mpu->fd[0], SMPLRT_DIV, div & 0xff);
    if (result < 0) {
        LOG_ERROR("wiringPiI2CWriteReg8(mpu->fd[0], SMPLRT_DIV, div & 0xff)");
        return result;
    }
    mpu_sleep(0.1);
    return result;
}

int
mpu_get_sample_rate_div(struct mpu *mpu)
{
    int result = 0;
    result = wiringPiI2CReadReg8(mpu->fd[0], SMPLRT_DIV) & 0xff;
    if (result < 0) {
        LOG_ERROR("wiringPiI2CReadReg8(mpu->fd[0], SMPLRT_DIV)");
        return result;
    }
    mpu_sleep(0.1);
    return result;
}

int
gyro_set_dlpf(struct mpu *mpu, int const val)
{
    int curr;
    int result;

    curr = wiringPiI2CReadReg8(mpu->fd[0], CONFIG);
    if (curr < 0) {
        LOG_ERROR("wiringPiI2CReadReg8(mpu->fd[0], CONFIG)");
        return curr;
    }
    mpu_sleep(0.1);
    curr |= (val & 0x07);
    result = wiringPiI2CWriteReg8(mpu->fd[0], CONFIG, curr & 0xff);
    if (result < 0) {
        LOG_ERROR("wiringPiI2CWriteReg8(mpu->fd[0], CONFIG, curr & 0xff)");
    }
    mpu_sleep(0.1);
    return result;
}

int
gyro_get_dlpf(struct mpu *mpu)
{
    int dlpf = wiringPiI2CReadReg8(mpu->fd[0], CONFIG) & 0x07;
    if (dlpf < 0) {
        LOG_ERROR("wiringPiI2CReadReg8(mpu->fd[0], CONFIG)");
        return dlpf;
    }
    return dlpf;
}

float
gyro_get_sample_rate(struct mpu *mpu)
{
    int div;
    int fchoice;
    div = mpu_get_sample_rate_div(mpu);
    if (div < 0) {
        LOG_ERROR("wiringPiI2CReadReg8(mpu->fd[0], SMPLRT_DIV)");
        return (float)div;
    }
    mpu_sleep(0.1);

    fchoice = wiringPiI2CReadReg8(mpu->fd[0], GYRO_CONFIG) & 0x03;
    if (fchoice < 0) {
        LOG_ERROR("wiringPiI2CReadReg8(mpu->fd[0], GYRO_CONFIG)");
        return (float)fchoice;
    }

    if ((fchoice & 0x01) == 1) {
        // int bandwidth = 8800;
        // float ms_delay = 0.064f;
        float freq = 32.f;
        float sample_rate = freq / (1.f + (float)div);
        mpu->conf.internal_sample_rate = freq;
        mpu->conf.sample_rate = sample_rate;
        return sample_rate;
    }

    if ((fchoice & 0x02) == 2) {
        // int bandwidth = 3600;
        // float ms_delay = 0.11f;
        float freq = 32.f;
        float sample_rate = freq / (1.f + (float)div);
        mpu->conf.internal_sample_rate = freq;
        mpu->conf.sample_rate = sample_rate;
        return sample_rate;
    }

    // int rate_idx = wiringPiI2CReadReg8(mpu->fd[0], CONFIG) & 0x07;
    int rate_idx = gyro_get_dlpf(mpu);
    /*
    float const ms_delays[] = {
        0.97f, 2.9f, 3.9f, 5.9f,
        9.9f, 17.85f, 33.48f, 0.17f
    };
    */
    // int const bandwidths[] = { 250, 184, 92, 41, 20, 10, 5, 3600 };
    int freqs[] = { 8, 1, 1, 1, 1, 1, 1, 8 };
    float internal_rate = (float)freqs[rate_idx];
    float sample_rate = internal_rate / (1.f + (float)div);
    mpu->conf.internal_sample_rate = internal_rate;
    mpu->conf.sample_rate = sample_rate;
    return sample_rate;

}

int
gyro_set_sens(struct mpu *mpu, enum fs_sel sel)
{
    float gyro_config_vals[] = { 250.f, 500.f, 1000.f, 2000.f }; // degrees/sec
    int result = 0;
    result = wiringPiI2CWriteReg8(
        mpu->fd[0],
        GYRO_CONFIG,
        (int)CONFIG_SEL[sel] & 0xff
        );
    if (result < 0) {
        LOG_ERROR("wiringPiI2CWriteReg8");
        return result;
    }
    mpu_sleep(0.1);
    mpu->conf.gyro_sens = gyro_config_vals[sel];
    return result;
}

int
accel_set_sens(struct mpu *mpu, enum fs_sel sel)
{
    float accel_config_vals[] = { 2.f, 4.f, 8.f, 16.f }; // g (g = 9.81 m/s^2)
    int result = 0;
    result = wiringPiI2CWriteReg8(
        mpu->fd[0],
        ACCEL_CONFIG,
        (int)CONFIG_SEL[sel] & 0xff
        );
    if (result < 0) {
        LOG_ERROR("wiringPiI2CWriteReg8");
        return result;
    }
    mpu_sleep(0.1);
    mpu->conf.accel_sens = accel_config_vals[sel];
    return result;
}


int
mpu_init(struct mpu *mpu)
{
    int result = 0;
    memset(mpu, 0, sizeof(*mpu));
    result = _mpu6050_init(mpu);
    result = _ak8963_init(mpu);
    return result;
}

struct vec3f
mpu_gyro(struct mpu *mpu)
{
    struct vec3f gyro = { .x = FLT_MAX, .y = FLT_MAX, .z = FLT_MAX };
    int gyro_x, gyro_y, gyro_z;
    // raw gyroscope bits
    if ((gyro_x = _read_raw_bits(mpu, GYRO_XOUT_H)) == INT_MAX) {
        LOG_ERROR("_read_raw_bits(mpu, GYRO_XOUT_H)");
        return gyro;
    }
    if ((gyro_y = _read_raw_bits(mpu, GYRO_YOUT_H)) == INT_MAX) {
        LOG_ERROR("_read_raw_bits(mpu, GYRO_YOUT_H)");
        return gyro;
    }
    if ((gyro_z = _read_raw_bits(mpu, GYRO_ZOUT_H)) == INT_MAX) {
        LOG_ERROR("_read_raw_bits(mpu, GYRO_ZOUT_H)");
        return gyro;
    }

    float w_x = ((float)gyro_x / MPU_DIV) * mpu->conf.gyro_sens;
    float w_y = ((float)gyro_y / MPU_DIV) * mpu->conf.gyro_sens;
    float w_z = ((float)gyro_z / MPU_DIV) * mpu->conf.gyro_sens;

    gyro.x = w_x;
    gyro.y = w_y;
    gyro.z = w_z;
    return gyro;
}

struct vec3f
mpu_accel(struct mpu *mpu)
{
    struct vec3f accel = { .x = FLT_MAX, .y = FLT_MAX, .z = FLT_MAX };
    int acc_x, acc_y, acc_z;
    // raw acceleration bits
    if ((acc_x = _read_raw_bits(mpu, ACCEL_XOUT_H)) == INT_MAX) {
        LOG_ERROR("_read_raw_bits(mpu, ACCEL_XOUT_H)");
        return accel;
    }

    if ((acc_y = _read_raw_bits(mpu, ACCEL_YOUT_H)) == INT_MAX) {
        LOG_ERROR("_read_raw_bits(mpu, ACCEL_YOUT_H)");
        return accel;
    }

    if ((acc_z = _read_raw_bits(mpu, ACCEL_ZOUT_H)) == INT_MAX) {
        LOG_ERROR("_read_raw_bits(mpu, ACCEL_ZOUT_H)");
        return accel;
    }

    // printf("%d %d %d\n", acc_x, acc_y, acc_z);

    // convert to acceleration in g and gyro dps
    float a_x = ((float)acc_x / MPU_DIV) * mpu->conf.accel_sens;
    float a_y = ((float)acc_y / MPU_DIV) * mpu->conf.accel_sens;
    float a_z = ((float)acc_z / MPU_DIV) * mpu->conf.accel_sens;
    // printf("%2.3f %2.3f %2.3f\n", a_x ,a_y, a_z);

    accel.x = a_x;
    accel.y = a_y;
    accel.z = a_z;

    return accel;
}


struct vec3f
mpu_mag(struct mpu *mpu)
{
    // raw magnetometer bits
    struct vec3f ret = { .x = FLT_MAX, .y = FLT_MAX, .z = FLT_MAX };

    int mag_x = 0;
    int mag_y = 0;
    int mag_z = 0;

    while (1) {
        if ((mag_x = _ak8963_read(mpu, HXH)) == INT_MAX) {
            LOG_ERROR("_ak8963_read(mpu, HXH)");
            return ret;
        }
        if ((mag_y = _ak8963_read(mpu, HYH)) == INT_MAX) {
            LOG_ERROR("_ak8963_read(mpu, HYH)");
            return ret;
        }
        if ((mag_z = _ak8963_read(mpu, HZH)) == INT_MAX) {
            LOG_ERROR("_ak8963_read(mpu, HZH)");
            return ret;
        }

        // the next line is needed for AK8963
        // if bin(wiringPiI2CReadReg8(AK8963_ADDR,AK8963_ST2))=='0b10000':
        if (((wiringPiI2CReadReg8(mpu->fd[1], AK8963_ST2)) & 0x08) != 0x08)
            break;
    }

    // convert to acceleration in g and gyro dps
    ret.x = _mag_convert(mag_x);
    ret.y = _mag_convert(mag_y);
    ret.z = _mag_convert(mag_z);

    return ret;
}



// gyro_sens, accel_sens = mpu6050_start() # instantiate gyro/accel
// mpu_sleep(0.1)

// #AK8963_coeffs = ak8963_start() # instantiate magnetometer
// mpu_sleep(0.1)

/*
int
main(void)
{
    int fd = 0;
    int result = 0;

    fd = wiringPiI2CSetup(MPU6050_ADDR);
    printf("Init result: %d\n", fd);

    result = wiringPiI2CWriteReg8(fd, PWR_MGMT_1, 0x80 & 0xff);
    if (result == -1)
        printf("Error: %d\n", errno);
    else
        printf("Success\n");

    result = wiringPiI2CWriteReg8(fd, PWR_MGMT_1, 0x00);
    if (result == -1)
        printf("Error: %d\n", errno);
    else
        printf("Success\n");

    return 0;
}

*/
