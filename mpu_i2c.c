
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mpu_debug.h"

// char const i2c_filename[] = "/dev/i2c-1";
// int const i2c_addr = 0x0c;

static inline int
i2c_rdwr_block(
    int fd,
    uint8_t reg,
    uint8_t read_write,
    uint8_t length,
    unsigned char *buffer
    )
{
    struct i2c_smbus_ioctl_data ioctl_data;
    union i2c_smbus_data smbus_data;

    int rv;

    if (length > I2C_SMBUS_BLOCK_MAX) {
        fprintf(
            stderr,
            "Requested length is greater than the maximum specified\n"
            );
        return -1;
    }

    smbus_data.block[0] = length;

    if (read_write != I2C_SMBUS_READ)
        for (int i = 0; i < length; ++i)
            smbus_data.block[i + 1] = buffer[i];

    ioctl_data.read_write = read_write;
    ioctl_data.command = reg;
    ioctl_data.size = I2C_SMBUS_I2C_BLOCK_DATA;
    ioctl_data.data = &smbus_data;

    rv = ioctl(fd, I2C_SMBUS, &ioctl_data);

    if (rv < 0) {
        fprintf(
            stderr,
            "Accessing I2C Read/Write failed: %s\n",
            strerror(errno)
            );
        return rv;
    }

    if (read_write == I2C_SMBUS_READ)
        for (int i = 0; i < length; ++i)
            buffer[i] = smbus_data.block[i + 1];

    return rv;
}

/*
static int
setup_i2c(char[] filename)
{
    int fd;
    int rv;

    if ((fd = open(filename, O_RDWR)) < 0) {
        fprintf(
            stderr,
            "Failed to open the i2c bus: %d\n",
            fd
            );
        return fd;
    }

    if ((rv = ioctl(fd, I2C_SLAVE, i2c_addr)) < 0) {
        fprintf(
            stderr,
            "Failed to acquire bus access and/or talk to slave: %d\n",
            rv
            );
        return rv;
    }

    return fd;
}
*/

int
i2c_read_block(int fd, int reg, int length, char *buffer)
{
    int rv;

    rv = i2c_rdwr_block(
        fd,
        (uint8_t)(reg & 0xff),
        I2C_SMBUS_READ,
        (uint8_t)length,
        (unsigned char *)buffer
        );
    if (rv < 0) {
        LOG_ERROR("Reading i2c bus error");
        return -1;
    }

    return 0;
}

int
i2c_write_block(int fd, int reg, int length, char *buffer)
{
    int rv;

    rv = i2c_rdwr_block(
        fd,
        (uint8_t)(reg & 0xff),
        I2C_SMBUS_WRITE,
        (uint8_t)length,
        (unsigned char *)buffer
        );
    if (rv < 0) {
        fprintf(stderr, "Writing i2c bus error\n");
        return -1;
    }

    return 0;
}


