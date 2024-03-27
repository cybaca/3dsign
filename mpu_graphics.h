#ifndef MPU_GRAPHICS_H_
#define MPU_GRAPHICS_H_

#include <GL/glu.h>
#include <GL/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
// #include "glad.h"

// void gluPerspective(double, double, double, double);


struct mpu_graphics {
    GLFWwindow *window;
    int width;
    int height;
};

void
mpu_cube_draw(void);

void
mpu_cube_draw_wire(void);

void
mpu_cube_rotate(float const x, float const y, float const z);

int
mpu_graphics_init(struct mpu_graphics *g);

void
mpu_graphics_loop(struct mpu_graphics *g, int style);

void
mpu_graphics_release(struct mpu_graphics *g);



#endif // MPU_GRAPHICS_H_
