#!/usr/bin/env python

import time
t0 = time.time()
start_bool = False
from sensors import *
while (time.time() - t0) < 5:
    try:
        from sensors import *
        start_bool = True
        break
    except:
        continue

imu_devs = ["ACCELEROMETER", "GYROSCOPE", "MAGNETOMETER"]
imu_labels = ["x-dir", "y-dir", "z-dir"]
imu_units = ["g", "g", "g", "dps", "dps", "dps", "uT", "uT", "uT"]


x_a = 0
y_a = 0
z_a = 0
x_g = 0
y_g = 0
z_g = 0

while 1:
    if start_bool == False:
        print("IMU not started, check wiring")
        break

    try:
        ax,ay,az,wx,wy,wz = mpu6050_conv() # read and convert mpu6050 data
        mx,my,mz = ak8963_conv() # read and convert ak8963 magnetometer data
    except:
        continue

    for imu_ii, imu_val in enumerate([ax,ay,az,wx,wy,wz]):
        if imu_ii % 3 == 0:
            print(20*"_" + "\n"+imu_devs[int(imu_ii/3)])
            print("{0}: {1:3.2f} {2}".format(
                imu_labels[imu_ii%3],
                imu_val,
                imu_units[imu_ii]))

    x_g += wx
    y_g += wy
    z_g += wz
    x_a += ax
    y_a += ay
    z_a += az
    #print('{}'.format('-'*30))
    #print('accel [g]: x = {0:2.2f}, y = {1:2.2f}, z = {2:2.2f}'.format(ax,ay,az))
    #print('gyro [dps]:  x = {0:2.2f}, y = {1:2.2f}, z = {2:2.2f}'.format(wx,wy,wz))
    #print('mag [uT]:   x = {0:2.2f}, y = {1:2.2f}, z = {2:2.2f}'.format(mx,my,mz))
    print('angle:  x = {0:2.2f}, y = {1:2.2f}, z = {2:2.2f}'.format(x_g,y_g,z_g))
    #print('{}'.format('-'*30))
    time.sleep(0.5)
