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
#include "cow.550"   // your existing cow array (must provide COWpoints, COWtris, COWntris, etc.)

const char* WINDOWTITLE = "Carousel Cow Animation";
const int INIT_WINDOW_SIZE = 1000;
const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;
const float MINSCALE = 0.05f;
const float BACKCOLOR[] = { 0.f, 0.f, 0.f, 1.f };

const int MS_PER_CYCLE = 10000;
float Time = 0.f;

enum ViewMode { OUTSIDE, INSIDE };
ViewMode CurrentView = OUTSIDE;

const float CAROUSEL_RADIUS = 2.f;
const int NUM_COWS = 4;

float Xrot = 0.f, Yrot = 0.f, Scale = 1.f;
int Xmouse, Ymouse, ActiveButton = 0;

GLuint AxesList = 0, CowList = 0, CircleList = 0, FloorList = 0, WallList = 0, TeapotList = 0, CubeList = 0;

bool FullLighting = true; // true = point light, false = spotlight
bool Frozen = false;

GLfloat LightColor[4] = { 1.f,1.f,1.f,1.f };

void Animate();
void Display();
void InitGraphics();
void InitLists();
void InitLighting();
void Reset();
void Keyboard(unsigned char, int, int);
void MouseButton(int, int, int, int);
void MouseMotion(int, int);
void SetMaterial(float r, float g, float b, float shininess);

float Ranf(float low, float high) {
    float r = (float)rand();
    return low + r / (float)RAND_MAX * (high - low);
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    InitGraphics();
    InitLighting();
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
    if (Frozen) return;
    int ms = glutGet(GLUT_ELAPSED_TIME);
    ms %= MS_PER_CYCLE;
    Time = (float)ms / (float)MS_PER_CYCLE;
    glutPostRedisplay();
}

void InitLists() {
    // --- Cow display list (from OBJ data) ---
    const float cowScale = 0.25f;
    const float cowHeightOffset = 1.1f * cowScale;

    CowList = glGenLists(1);
    glNewList(CowList, GL_COMPILE);
    glPushMatrix();
    glRotatef(-90.f, 0.f, 1.f, 0.f);   // orient the OBJ
    glTranslatef(0.f, cowHeightOffset, 0.f);
    glScalef(cowScale, cowScale, cowScale);

    // The cow geometry: emit normals per triangle (flat shading)
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
        float len = sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        if (len != 0.f) { n[0] /= len;n[1] /= len;n[2] /= len; }
        glNormal3f(n[0], n[1], n[2]);
        glVertex3f(p0.x, p0.y, p0.z);
        glVertex3f(p1.x, p1.y, p1.z);
        glVertex3f(p2.x, p2.y, p2.z);
    }
    glEnd();

    glPopMatrix();
    glEndList();

    // --- Axes list ---
    AxesList = glGenLists(1);
    glNewList(AxesList, GL_COMPILE);
    glBegin(GL_LINES);
    glColor3f(1.f, 0.f, 0.f); glVertex3f(0.f, 0.f, 0.f); glVertex3f(1.f, 0.f, 0.f);
    glColor3f(0.f, 1.f, 0.f); glVertex3f(0.f, 0.f, 0.f); glVertex3f(0.f, 1.f, 0.f);
    glColor3f(0.f, 0.f, 1.f); glVertex3f(0.f, 0.f, 0.f); glVertex3f(0.f, 0.f, 1.f);
    glEnd();
    glEndList();

    // --- Circle list for carousel ---
    CircleList = glGenLists(1);
    glNewList(CircleList, GL_COMPILE);
    glColor3f(1.f, 0.f, 0.f);
    glBegin(GL_LINE_LOOP);
    const int segments = 100;
    for (int i = 0;i < segments;i++) {
        float theta = 2.f * (float)M_PI * i / segments;
        glVertex3f(CAROUSEL_RADIUS * cosf(theta), 0.f, CAROUSEL_RADIUS * sinf(theta));
    }
    glEnd();
    glEndList();

    // --- Floor ---
    FloorList = glGenLists(1);
    glNewList(FloorList, GL_COMPILE);
    const float floorSize = 10.f;
    const int tiles = 40;
    const float half = floorSize * 0.5f;
    const float tileSize = floorSize / (float)tiles;
    for (int i = 0;i < tiles;i++) {
        float z0 = -half + i * tileSize;
        float z1 = z0 + tileSize;
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0;j <= tiles;j++) {
            float x = -half + j * tileSize;
            glNormal3f(0.f, 1.f, 0.f);
            glVertex3f(x, 0.f, z0);
            glNormal3f(0.f, 1.f, 0.f);
            glVertex3f(x, 0.f, z1);
        }
        glEnd();
    }
    glEndList();

    // --- Walls ---
    WallList = glGenLists(1);
    glNewList(WallList, GL_COMPILE);
    const float wallHalf = 5.f;
    const float wallHeight = 3.f;
    const int wallTiles = 20;
    const float tileX = (wallHalf * 2.f) / (float)wallTiles;
    const float tileY = wallHeight / (float)wallTiles;
    float z = -wallHalf;
    for (int i = 0;i < wallTiles;i++) {
        float y0 = i * tileY, y1 = y0 + tileY;
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0;j <= wallTiles;j++) {
            float x = -wallHalf + j * tileX;
            glNormal3f(0.f, 0.f, 1.f);
            glVertex3f(x, y0, z);
            glNormal3f(0.f, 0.f, 1.f);
            glVertex3f(x, y1, z);
        }
        glEnd();
    }
    float x = -wallHalf;
    for (int i = 0;i < wallTiles;i++) {
        float y0 = i * tileY, y1 = y0 + tileY;
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0;j <= wallTiles;j++) {
            float zpos = -wallHalf + j * tileX;
            glNormal3f(1.f, 0.f, 0.f);
            glVertex3f(x, y0, zpos);
            glNormal3f(1.f, 0.f, 0.f);
            glVertex3f(x, y1, zpos);
        }
        glEnd();
    }
    glEndList();

    // --- Teapot ---
    TeapotList = glGenLists(1);
    glNewList(TeapotList, GL_COMPILE);
    glutSolidTeapot(0.4);
    glEndList();

    // --- Cube ---
    CubeList = glGenLists(1);
    glNewList(CubeList, GL_COMPILE);
    glutSolidCube(0.6);
    glEndList();
}

void Display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.f, 1.f, 0.1f, 1000.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (CurrentView == OUTSIDE) {
        gluLookAt(5.f, 3.f, 5.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
        glRotatef(Xrot, 1.f, 0.f, 0.f);
        glRotatef(Yrot, 0.f, 1.f, 0.f);
        glScalef(Scale, Scale, Scale);
    }
    else {
        gluLookAt(0.f, 3.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    // Floor
    SetMaterial(0.6f, 0.8f, 0.6f, 5.f);
    glCallList(FloorList);

    // Walls
    SetMaterial(0.6f, 0.7f, 0.9f, 5.f);
    glCallList(WallList);

    // Cows
    SetMaterial(0.8f, 0.6f, 0.4f, 10.f);
    for (int i = 0;i < NUM_COWS;i++) {
        float phase = i * 0.25f;
        float angle = 2.f * (float)M_PI * (Time + phase);
        float x = CAROUSEL_RADIUS * cosf(angle);
        float z = CAROUSEL_RADIUS * sinf(angle);
        float y = 0.2f * sinf(2.f * (float)M_PI * (Time + phase));
        float rock = 15.f * sinf(4.f * (float)M_PI * (Time + phase));
        glPushMatrix();
        glTranslatef(x, y, z);
        glRotatef(-angle * 180.f / (float)M_PI, 0.f, 1.f, 0.f);
        glRotatef(rock, 1.f, 0.f, 0.f);
        glCallList(CowList);
        glPopMatrix();
    }

    // Teapot
    SetMaterial(0.1f, 0.2f, 0.9f, 80.f);
    glPushMatrix();
    glTranslatef(1.0f, 0.5f, 0.f);
    glCallList(TeapotList);
    glPopMatrix();

    // Cube
    SetMaterial(0.9f, 0.2f, 0.2f, 5.f);
    glPushMatrix();
    glTranslatef(-0.8f, 0.4f, -0.8f);
    glCallList(CubeList);
    glPopMatrix();

    // --- Light animation ---
    const float LIGHTRADIUS = 4.f;
    float theta = 2.f * (float)M_PI * Time;
    float xlight = LIGHTRADIUS * cosf(theta);
    float zlight = LIGHTRADIUS * sinf(theta);
    float ylight = 3.f;

    if (FullLighting) {
        GLfloat pos[] = { xlight,ylight,zlight,1.f };
        glLightfv(GL_LIGHT0, GL_POSITION, pos);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, LightColor);
        glLightfv(GL_LIGHT0, GL_SPECULAR, LightColor);
        glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.f);
    }
    else {
        GLfloat pos[] = { xlight,ylight,zlight,1.f };
        GLfloat dir[] = { -xlight, -ylight + 1.f, -zlight };
        float len = sqrtf(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
        dir[0] /= len; dir[1] /= len; dir[2] /= len;
        glLightfv(GL_LIGHT0, GL_POSITION, pos);
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, LightColor);
        glLightfv(GL_LIGHT0, GL_SPECULAR, LightColor);
        glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.f);
    }

    // --- Light sphere (unlit) ---
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(xlight, ylight, zlight);
    glColor3fv(LightColor);
    glutSolidSphere(0.15, 20, 20);
    glPopMatrix();

    glCallList(AxesList);

    glutSwapBuffers();
}

void InitGraphics() {
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);
    glutCreateWindow(WINDOWTITLE);
    glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
}

void InitLighting() {
    GLfloat ambient[] = { 0.2f,0.2f,0.2f,1.f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void Reset() {
    Xrot = Yrot = 0.f;
    Scale = 1.f;
    ActiveButton = 0;
    CurrentView = OUTSIDE;
    FullLighting = true;
    Frozen = false;
}

void Keyboard(unsigned char c, int, int) {
    switch (c) {
    case 'o': case 'O': CurrentView = OUTSIDE; break;
    case 'i': case 'I': CurrentView = INSIDE; break;
    case 27: exit(0);
    case 'f': case 'F':
        Frozen = !Frozen;
        if (Frozen) glutIdleFunc(NULL);
        else glutIdleFunc(Animate);
        break;
    case 'p': case 'P': FullLighting = true; break;
    case 's': case 'S': FullLighting = false; break;
    case 'w': LightColor[0] = 1.f; LightColor[1] = 1.f; LightColor[2] = 1.f; break;
    case 'r': LightColor[0] = 1.f; LightColor[1] = 0.f; LightColor[2] = 0.f; break;
    case 't': LightColor[0] = 1.f; LightColor[1] = 0.5f; LightColor[2] = 0.f; break;
    case 'y': LightColor[0] = 1.f; LightColor[1] = 1.f; LightColor[2] = 0.f; break;
    case 'g': LightColor[0] = 0.f; LightColor[1] = 1.f; LightColor[2] = 0.f; break;
    case 'c': LightColor[0] = 0.f; LightColor[1] = 1.f; LightColor[2] = 1.f; break;
    case 'm': LightColor[0] = 1.f; LightColor[1] = 0.f; LightColor[2] = 1.f; break;
    }
    glutPostRedisplay();
}

void MouseButton(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        Xmouse = x; Ymouse = y;
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
    if (ActiveButton & 1) { Xrot += dy * ANGFACT; Yrot += dx * ANGFACT; }
    if (ActiveButton & 2) { Scale += (dx - dy) * SCLFACT; if (Scale < MINSCALE) Scale = MINSCALE; }
    Xmouse = x; Ymouse = y;
    glutPostRedisplay();
}

void SetMaterial(float r, float g, float b, float shininess) {
    GLfloat ambient[] = { 0.15f * r,0.15f * g,0.15f * b,1.f };
    GLfloat diffuse[] = { 0.6f * r,0.6f * g,0.6f * b,1.f };
    GLfloat specular[] = { 0.3f,0.3f,0.3f,1.f };
    if (shininess > 50.f) specular[0] = specular[1] = specular[2] = 0.9f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}
