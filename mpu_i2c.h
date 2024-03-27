#ifndef MPU_I2C_H_
#define MPU_I2C_H_

int
i2c_read_block(int fd, int reg, int length, char *buffer);

int
i2c_write_block(int fd, int reg, int length, char *buffer);



#endif // MPU_I2C_H_
