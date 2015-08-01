#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "pattern.h"
#include "layout.h"

const int width = 800;
SDL_Window * win;

#define CIRCLE_RADIUS .012
#define CIRCLE_POINTS 24

void draw_circle(double x, double y, rgb_t color) {
    int i;
    glColor3ub(color.r, color.g, color.b);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (i = 0; i <= CIRCLE_POINTS; i++) {
        glVertex2f(cos(M_PI * 2 * i / CIRCLE_POINTS) * CIRCLE_RADIUS,
                   sin(M_PI * 2 * i / CIRCLE_POINTS) * CIRCLE_RADIUS);
    }
    glEnd();
    glPopMatrix();
}

int main() {
    int t;
    double z;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        exit(1);
    }

    win = SDL_CreateWindow("trees",
                           SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                           width, width,
                           SDL_WINDOW_OPENGL);

    SDL_GL_CreateContext(win);

    glEnable(GL_TEXTURE_2D);
    glViewport(0, 0, width, width);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, width, width, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(width / 2, width / 2, 0);
    glScalef(width / 2, width / -2, 0);

    t = SDL_GetTicks();

    z = 0;

    while (!SDL_QuitRequested()) {
        int t2;
        size_t i;

        /* Handle SDL events */
        SDL_Event event;
        while (SDL_PollEvent(&event)) { }

        glColor3ub(0, 0, 0);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(-2, -2);
        glVertex2f(-2, 2);
        glVertex2f(2, 2);
        glVertex2f(2, -2);
        glEnd();

        for (i = 0; i < sizeof(tree_centers)/sizeof(tree_centers[0]); i++) {
            rgb_t pt = pattern_arr[1].func(0, i, z);
            draw_circle(tree_centers[i].x, tree_centers[i].y, pt);
        }

        SDL_GL_SwapWindow(win);

        t2 = SDL_GetTicks();
        if (t2 < t+20) {
            t += 20;
            SDL_Delay(t - t2);
        } else {
            SDL_Delay(1);
            t = t2;
        }

        z += .04;
    }

    return 0;
}



