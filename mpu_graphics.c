#define GLFW_INCLUDE_NONE
// #include <GLFW/glfw3.h>
// #include "glad.h"
#include <stdio.h>
#include <string.h>
#include "mpu_time.h"
#include "mpu_graphics.h"


static GLfloat const cube_vert[8][3] = {
    { 1.f, 1.f, 1.f },
    { 1.f, 1.f, -1.f },
    { 1.f, -1.f, -1.f },
    { 1.f, -1.f, 1.f },
    { -1.f, 1.f, 1.f },
    { -1.f, -1.f, -1.f },
    { -1.f, -1.f, 1.f },
    { -1.f, 1.f, -1.f },
};

static GLint const cube_edge[12][2] = {
    { 0, 1 }, { 0, 3 }, { 0, 4 }, 
    { 1, 2 }, { 1, 7 }, { 2, 5 }, 
    { 2, 3 }, { 3, 6 }, { 4, 6 }, 
    { 4, 7 }, { 5, 6 }, { 5, 7 }
};

static GLint const cube_quad[6][4] = {
    { 0, 3, 6, 4 }, { 2, 5, 6, 3 }, 
    { 1, 2, 5, 7 }, { 1, 0, 4, 7 }, 
    { 7, 4, 6, 5 }, { 2, 3, 0, 1 }
};

static void
error_callback(int error, const char *desc)
{
    fprintf(stderr, "Error %d: %s\n", error, desc);
}

static void
key_callback(
    GLFWwindow *window,
    int key,
    __attribute__ ((unused)) int scancode,
    int action,
    __attribute__ ((unused)) int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void
mpu_cube_draw_wire(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBegin(GL_LINES);
    int i;
    for (i = 0; i < 12; ++i) {
        GLint const *edge = cube_edge[i];
        GLint a = edge[0];
        GLint b = edge[1];
        glVertex3fv(cube_vert[a]);
        glVertex3fv(cube_vert[b]);
    }

    glEnd();
    // glPopMatrix();
    mpu_sleep(0.1f);
}

void
mpu_cube_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBegin(GL_QUADS);
    int i;
    for (i = 0; i < 6; ++i) {
        GLint const *quad = cube_quad[i];
        GLint a = quad[0];
        GLint b = quad[1];
        GLint c = quad[2];
        GLint d = quad[3];
        glVertex3fv(cube_vert[a]);
        glVertex3fv(cube_vert[b]);
        glVertex3fv(cube_vert[c]);
        glVertex3fv(cube_vert[d]);
    }
    glEnd();
    glPopMatrix();
    mpu_sleep(0.1f);
}


void
mpu_cube_rotate(float const x, float const y, float const z)
{
    // glPushMatrix();
    glRotatef(x, 1.f, 0.f, 0.f);
    glRotatef(y, 0.f, 1.f, 0.f);
    glRotatef(z, 0.f, 0.f, 1.f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int
mpu_graphics_init(struct mpu_graphics *g)
{
    GLFWwindow *window = NULL;
    int width, height;
    memset(g, 0, sizeof(*g));

    if (!glfwInit()) {
        fprintf(stderr, "glfwInit failure\n");
        return 1;
    }

    if ((window = glfwCreateWindow(640, 480, "MPU", NULL, NULL)) == NULL) {
        fprintf(stderr, "glfwCreateWindow failure\n");
        return 1;
    }

    glfwMakeContextCurrent(window);

    glfwSetErrorCallback(error_callback);
    glfwSetKeyCallback(window, key_callback);

    // gladLoadGL();

    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSwapInterval(1);

    /*
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    */

    // glfwDestroyWindow(window);
    // glfwTerminate();

    g->window = window;
    g->width = width;
    g->height = height;

    return 0;
}

void
mpu_graphics_loop(struct mpu_graphics *g, int style)
{
    if (style == 0)
        while (!glfwWindowShouldClose(g->window)) {
            glfwPollEvents();
            mpu_cube_draw_wire();
            glfwSwapBuffers(g->window);
        }
    if (style == 1)
        while (!glfwWindowShouldClose(g->window)) {
            glfwPollEvents();
            mpu_cube_draw();
            glfwSwapBuffers(g->window);
        }
}

void
mpu_graphics_release(struct mpu_graphics *g)
{
    if (g->window != NULL)
        glfwDestroyWindow(g->window);
    glfwTerminate();
    memset(g, 0, sizeof(*g));
}


