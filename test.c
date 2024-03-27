#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "mpu_graphics.h"
#include "mpu_array.h"
#include "mpu_time.h"
#include "mpu_integral.h"
#include "mpu_linear.h"
#include "mpu.h"



void
print_vec(struct vec3f *v)
{
    printf("x: %2.3f, y: %2.3f, z: %2.3f\n", v->x, v->y, v->z);
}


struct vec3f
rotation_test(void)
{
    struct mpu mpu;
    mpu_init(&mpu);

    struct array_f32 xarr;
    struct array_f32 yarr;
    struct array_f32 zarr;
    struct array_f32 tarr;
    array_f32_init(&xarr);
    array_f32_init(&yarr);
    array_f32_init(&zarr);
    array_f32_init(&tarr);

    float t0 = mpu_seconds();
    float seconds = 0.f;

    printf("start rotating\n");
    mpu_sleep(0.5f);
    printf("now\n");

    while (seconds < 5.f) {
        struct vec3f w = mpu_gyro(&mpu);
        seconds = mpu_seconds() - t0;
        array_f32_append(&xarr, w.x);
        array_f32_append(&yarr, w.y);
        array_f32_append(&zarr, w.z);
        array_f32_append(&tarr, seconds);
    }

    int size = (int)array_f32_size(&xarr);
    float samp_rate = (float)size / (tarr.arr[size - 1] - tarr.arr[0]);

    assert(array_f32_size(&xarr) == array_f32_size(&tarr));
    assert(array_f32_size(&yarr) == array_f32_size(&tarr));
    assert(array_f32_size(&zarr) == array_f32_size(&tarr));

    /*
    int new_size_x = 0;
    int new_size_y = 0;
    int new_size_z = 0;
    float *rotx = rot_displace(
        array_f32_data(&xarr),
        array_f32_data(&tarr),
        (int)array_f32_size(&xarr),
        &new_size_x
        );
    float *roty = rot_displace(
        array_f32_data(&xarr),
        array_f32_data(&tarr),
        (int)array_f32_size(&yarr),
        &new_size_y
        );
    float *rotz = rot_displace(
        array_f32_data(&zarr),
        array_f32_data(&tarr),
        (int)array_f32_size(&zarr),
        &new_size_z
        );

    struct vec3f ret;
    ret.x = xarr.arr[new_size_x - 1];
    ret.y = yarr.arr[new_size_y - 1];
    ret.z = zarr.arr[new_size_z - 1];
    */

    /*
    struct vec3f ret;
    ret.x = total_displacement(
        array_f32_data(&xarr),
        array_f32_data(&tarr),
        (int)array_f32_size(&xarr)

        );
    ret.y = total_displacement(
        array_f32_data(&yarr),
        array_f32_data(&tarr),
        (int)array_f32_size(&yarr)
        );
    ret.z = total_displacement(
        array_f32_data(&zarr),
        array_f32_data(&tarr),
        (int)array_f32_size(&zarr)
        );
    */

    struct vec3f ret;
    ret.x = trapz_displacement(
        array_f32_data(&xarr),
        array_f32_data(&tarr),
        (int)array_f32_size(&xarr)

        );
    ret.y = trapz_displacement(
        array_f32_data(&yarr),
        array_f32_data(&tarr),
        (int)array_f32_size(&yarr)
        );
    ret.z = trapz_displacement(
        array_f32_data(&zarr),
        array_f32_data(&tarr),
        (int)array_f32_size(&zarr)
        );

    array_f32_release(&xarr);
    array_f32_release(&yarr);
    array_f32_release(&zarr);
    /*
    free(rotx);
    free(roty);
    free(rotz);
    */
        
    printf("sample rate: %f\n", samp_rate);
    print_vec(&ret);
    return ret;
}

void
test(void)
{

    struct mpu mpu;
    mpu_init(&mpu);

    while (1) {
        struct vec3f a = mpu_accel(&mpu);
        struct vec3f w = mpu_gyro(&mpu);
        // struct vec3f g = mpu_mag(&mpu);

        printf("--------------------------------------------------------\n");
        printf("Acceleration:\n");
        print_vec(&a);
        printf("Angular velocity:\n");
        print_vec(&w);
        // printf("Gravity: \n");
        // print_vec(&g);

        mpu_sleep(0.4);
    }
}

void
graphics_test(void)
{
    struct mpu_graphics g;
    if (mpu_graphics_init(&g) != 0)
        return;

    struct mpu mpu;
    mpu_init(&mpu);

    struct array_f32 xarr;
    struct array_f32 yarr;
    struct array_f32 zarr;
    struct array_f32 tarr;
    array_f32_init(&xarr);
    array_f32_init(&yarr);
    array_f32_init(&zarr);
    array_f32_init(&tarr);

    float t0 = 0.f; // mpu_seconds();
    float seconds = 0.f;

    printf("start rotating\n");
    mpu_sleep(0.5f);
    printf("now\n");

    gluPerspective(45, 640 / 480, 0.1, 50.0);
    glTranslatef(0.f, 0.f, -10.f);
    while (!glfwWindowShouldClose(g.window)) {

        t0 = mpu_seconds();
        seconds = 0.f;

        glfwPollEvents();
        while (seconds < 0.1f) {
            struct vec3f w = mpu_gyro(&mpu);
            seconds = mpu_seconds() - t0;
            array_f32_append(&xarr, w.x);
            array_f32_append(&yarr, w.y);
            array_f32_append(&zarr, w.z);
            array_f32_append(&tarr, seconds);
        }

        int size = (int)array_f32_size(&xarr);
        float samp_rate = (float)size / (tarr.arr[size - 1] - tarr.arr[0]);

        assert(array_f32_size(&xarr) == array_f32_size(&tarr));
        assert(array_f32_size(&yarr) == array_f32_size(&tarr));
        assert(array_f32_size(&zarr) == array_f32_size(&tarr));

        struct vec3f ret;
        ret.x = trapz_displacement(
            array_f32_data(&xarr),
            array_f32_data(&tarr),
            (int)array_f32_size(&xarr)

            );
        ret.y = trapz_displacement(
            array_f32_data(&yarr),
            array_f32_data(&tarr),
            (int)array_f32_size(&yarr)
            );
        ret.z = trapz_displacement(
            array_f32_data(&zarr),
            array_f32_data(&tarr),
            (int)array_f32_size(&zarr)
            );

        printf("sample rate: %f\n", samp_rate);
        print_vec(&ret);

        mpu_sleep(0.1f);
        mpu_cube_rotate(ret.x, ret.y, ret.z);
        mpu_cube_draw_wire();
        glfwSwapBuffers(g.window);

        array_f32_clear(&xarr);
        array_f32_clear(&yarr);
        array_f32_clear(&zarr);
        array_f32_clear(&tarr);

        mpu_sleep(0.1f);

    }

    array_f32_release(&xarr);
    array_f32_release(&yarr);
    array_f32_release(&zarr);
        

    mpu_graphics_release(&g);
}

/*
void
graphics_test(void)
{
    struct mpu_graphics g;

    if (mpu_graphics_init(&g) != 0)
        return;




    gluPerspective(45., 640. / 480., 0.1, 50.0);
    glTranslatef(0.f, 0.f, -10.f);
    while (!glfwWindowShouldClose(g.window)) {
        glfwPollEvents();

        glRotatef(1.f, 3.f, 1.f, 1.f);
        mpu_cube_draw_wire();
        glfwSwapBuffers(g.window);
    }

    mpu_graphics_release(&g);
}
*/
int
main(void)
{
    // rotation_test();
    graphics_test();
    return 0;
}
