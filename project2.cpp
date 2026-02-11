#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctime>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include "glut.h"

#include "cow.550"

const char* WINDOWTITLE = "Carousel Cow Animation";
const int INIT_WINDOW_SIZE = 1000;
const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;
const float MINSCALE = 0.05f;
const float BACKCOLOR[] = { 0.,0.,0.,1. };

const int MS_PER_CYCLE = 10000;
float Time = 0.;

enum ViewMode { OUTSIDE, INSIDE };
ViewMode CurrentView = OUTSIDE;

const float CAROUSEL_RADIUS = 2.f;
const int NUM_COWS = 4;

float Xrot = 0., Yrot = 0., Scale = 1.f;
int Xmouse, Ymouse, ActiveButton = 0;

GLuint AxesList, CowList, CircleList;

void Animate();
void Display();
void InitGraphics();
void InitLists();
void Reset();
void Keyboard(unsigned char, int, int);
void MouseButton(int, int, int, int);
void MouseMotion(int, int);

float Ranf(float low, float high) {
    float r = (float)rand();
    return low + r / RAND_MAX * (high - low);
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    InitGraphics();
    InitLists();
    Reset();

    glutDisplayFunc(Display);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(MouseButton);
    glutMotionFunc(MouseMotion);
    glutIdleFunc(Animate);
    glutMainLoop();
    return 0;
}

void Animate() {
    int ms = glutGet(GLUT_ELAPSED_TIME);
    ms %= MS_PER_CYCLE;
    Time = (float)ms / (float)MS_PER_CYCLE;
    glutPostRedisplay();
}

void InitLists() {
    const float cowScale = 0.25f;
    const float cowHeightOffset = 1.1f * cowScale;

    CowList = glGenLists(1);
    glNewList(CowList, GL_COMPILE);
    glPushMatrix();
    glRotatef(-90.f, 0.f, 1.f, 0.f);  
    glTranslatef(0.f, cowHeightOffset, 0.f);
    glScalef(cowScale, cowScale, cowScale);

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < COWntris; i++) {
        struct point p0 = COWpoints[COWtris[i].p0];
        struct point p1 = COWpoints[COWtris[i].p1];
        struct point p2 = COWpoints[COWtris[i].p2];

        float u[3] = { p1.x - p0.x, p1.y - p0.y, p1.z - p0.z };
        float v[3] = { p2.x - p0.x, p2.y - p0.y, p2.z - p0.z };
        float n[3] = {
            u[1] * v[2] - u[2] * v[1],
            u[2] * v[0] - u[0] * v[2],
            u[0] * v[1] - u[1] * v[0]
        };
        float len = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        if (len != 0.f) { n[0] /= len; n[1] /= len; n[2] /= len; }
        float brightness = 0.5f + 0.5f * n[1];
        glColor3f(brightness, brightness, 0.f);

        glVertex3f(p0.x, p0.y, p0.z);
        glVertex3f(p1.x, p1.y, p1.z);
        glVertex3f(p2.x, p2.y, p2.z);
    }
    glEnd();

    glPopMatrix();
    glEndList();

    AxesList = glGenLists(1);
    glNewList(AxesList, GL_COMPILE);
    glBegin(GL_LINES);
    glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(1, 0, 0);
    glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 1, 0);
    glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 1);
    glEnd();
    glEndList();

    CircleList = glGenLists(1);
    glNewList(CircleList, GL_COMPILE);
    glColor3f(1.f, 0.f, 0.f);
    glBegin(GL_LINE_LOOP);
    const int segments = 100;
    for (int i = 0; i < segments; i++) {
        float theta = 2.f * M_PI * i / segments;
        glVertex3f(CAROUSEL_RADIUS * cos(theta), 0.f, CAROUSEL_RADIUS * sin(theta));
    }
    glEnd();
    glEndList();
}

void Display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70., 1., 0.1, 1000.);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (CurrentView == OUTSIDE) {
        gluLookAt(5., 3., 5., 0., 0., 0., 0., 1., 0.);
        glRotatef(Xrot, 1., 0., 0.);
        glRotatef(Yrot, 0., 1., 0.);
        glScalef(Scale, Scale, Scale);
    }
    else {
        gluLookAt(0., 3., 0., 0., 0., 1., 0., 1., 0.);
    }

    glEnable(GL_DEPTH_TEST);

    glCallList(AxesList);
    glCallList(CircleList);

    for (int i = 0; i < NUM_COWS; i++) {
        float phase = i * 0.25f;
        float angle = 2.f * M_PI * (Time + phase);
        float x = CAROUSEL_RADIUS * cos(angle);
        float z = CAROUSEL_RADIUS * sin(angle);
        float y = 0.2f * sin(2.f * M_PI * (Time + phase));
        float rock = 15.f * sin(4.f * M_PI * (Time + phase));

        glPushMatrix();
        glTranslatef(x, y, z);
        glRotatef(-angle * 180.f / M_PI, 0., 1., 0.);
        glRotatef(rock, 1., 0., 0.);
        glCallList(CowList);
        glPopMatrix();
    }

    glutSwapBuffers();
}

void InitGraphics() {
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);
    glutCreateWindow(WINDOWTITLE);
    glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);
    glEnable(GL_DEPTH_TEST);
}

void Reset() {
    Xrot = Yrot = 0.;
    Scale = 1.f;
    ActiveButton = 0;
    CurrentView = OUTSIDE;
}

void Keyboard(unsigned char c, int, int) {
    switch (c) {
    case 'o': case 'O': CurrentView = OUTSIDE; break;
    case 'i': case 'I': CurrentView = INSIDE; break;
    case 27: exit(0);
    }
    glutPostRedisplay();
}

void MouseButton(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        Xmouse = x;
        Ymouse = y;
        if (button == GLUT_LEFT_BUTTON) ActiveButton |= 1;
        if (button == GLUT_MIDDLE_BUTTON) ActiveButton |= 2;
    }
    else {
        if (button == GLUT_LEFT_BUTTON) ActiveButton &= ~1;
        if (button == GLUT_MIDDLE_BUTTON) ActiveButton &= ~2;
    }
}

void MouseMotion(int x, int y) {
    int dx = x - Xmouse;
    int dy = y - Ymouse;

    if (ActiveButton & 1) {
        Xrot += dy * ANGFACT;
        Yrot += dx * ANGFACT;
    }
    if (ActiveButton & 2) {
        Scale += (dx - dy) * SCLFACT;
        if (Scale < MINSCALE) Scale = MINSCALE;
    }

    Xmouse = x;
    Ymouse = y;
    glutPostRedisplay();
}
