#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <vector>
#include <string>

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

int currentObject = 0;
float camAngleX = 45.0f, camAngleY = 30.0f;
int lastMouseX, lastMouseY;
bool dragging = false;
GLuint textures[6];
float lightAngle = 0.0f;
float cowRotation = 0.0f;
GLuint CowList;

void Mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        dragging = (state == GLUT_DOWN);
        lastMouseX = x;
        lastMouseY = y;
    }
}
void Motion(int x, int y) {
    if (dragging) {
        camAngleX += (x - lastMouseX) * 0.5f;
        camAngleY += (y - lastMouseY) * 0.5f;
        if (camAngleY > 89.0f) camAngleY = 89.0f;
        if (camAngleY < -89.0f) camAngleY = -89.0f;
        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
}
void Keyboard(unsigned char key, int x, int y) {
    if (key >= '0' && key <= '5') { currentObject = key - '0'; glutPostRedisplay(); }
    if (key == 27) exit(0);
}
void Idle() {
    lightAngle += 0.01f;
    if (lightAngle > 2 * M_PI) lightAngle -= 2 * M_PI;
    cowRotation += 0.5f;
    glutPostRedisplay();
}

GLuint LoadBMP(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) { printf("Cannot open %s\n", filename); return 0; }
    unsigned char info[54]; fread(info, sizeof(unsigned char), 54, f);
    int w = *(int*)&info[18]; int h = *(int*)&info[22]; int size = 3 * w * h;
    unsigned char* data = (unsigned char*)malloc(size); fread(data, sizeof(unsigned char), size, f); fclose(f);
    for (int i = 0;i < size;i += 3) { unsigned char tmp = data[i]; data[i] = data[i + 2]; data[i + 2] = tmp; }
    GLuint tex; glGenTextures(1, &tex); glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data); free(data);
    return tex;
}

void InitCow() {
    const float cowScale = 0.25f;
    const float cowHeightOffset = 1.1f * cowScale;

    CowList = glGenLists(1);
    glNewList(CowList, GL_COMPILE);
    glPushMatrix();
    glRotatef(-90.f, 0.f, 1.f, 0.f);
    glTranslatef(0.f, cowHeightOffset, 0.f);
    glScalef(cowScale, cowScale, cowScale);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[5]);

    glBegin(GL_TRIANGLES);
    for (int i = 0;i < COWntris;i++) {
        struct point p0 = COWpoints[COWtris[i].p0];
        struct point p1 = COWpoints[COWtris[i].p1];
        struct point p2 = COWpoints[COWtris[i].p2];

        float u[3] = { p1.x - p0.x,p1.y - p0.y,p1.z - p0.z };
        float v[3] = { p2.x - p0.x,p2.y - p0.y,p2.z - p0.z };
        float n[3] = { u[1] * v[2] - u[2] * v[1], u[2] * v[0] - u[0] * v[2], u[0] * v[1] - u[1] * v[0] };
        float len = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        if (len != 0.f) { n[0] /= len; n[1] /= len; n[2] /= len; }
        glNormal3f(n[0], n[1], n[2]);

        glTexCoord2f(0.5f + p0.x, 0.5f + p0.y); glVertex3f(p0.x, p0.y, p0.z);
        glTexCoord2f(0.5f + p1.x, 0.5f + p1.y); glVertex3f(p1.x, p1.y, p1.z);
        glTexCoord2f(0.5f + p2.x, 0.5f + p2.y); glVertex3f(p2.x, p2.y, p2.z);
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glEndList();
}

void DrawTexturedCube(GLuint tex) {
    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-1, -1, 1);
    glTexCoord2f(1, 0); glVertex3f(1, -1, 1);
    glTexCoord2f(1, 1); glVertex3f(1, 1, 1);
    glTexCoord2f(0, 1); glVertex3f(-1, 1, 1);
    glTexCoord2f(0, 0); glVertex3f(1, -1, -1);
    glTexCoord2f(1, 0); glVertex3f(-1, -1, -1);
    glTexCoord2f(1, 1); glVertex3f(-1, 1, -1);
    glTexCoord2f(0, 1); glVertex3f(1, 1, -1);
    glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
    glTexCoord2f(1, 0); glVertex3f(-1, -1, 1);
    glTexCoord2f(1, 1); glVertex3f(-1, 1, 1);
    glTexCoord2f(0, 1); glVertex3f(-1, 1, -1);
    glTexCoord2f(0, 0); glVertex3f(1, -1, 1);
    glTexCoord2f(1, 0); glVertex3f(1, -1, -1);
    glTexCoord2f(1, 1); glVertex3f(1, 1, -1);
    glTexCoord2f(0, 1); glVertex3f(1, 1, 1);
    glTexCoord2f(0, 0); glVertex3f(-1, 1, 1);
    glTexCoord2f(1, 0); glVertex3f(1, 1, 1);
    glTexCoord2f(1, 1); glVertex3f(1, 1, -1);
    glTexCoord2f(0, 1); glVertex3f(-1, 1, -1);
    glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
    glTexCoord2f(1, 0); glVertex3f(1, -1, -1);
    glTexCoord2f(1, 1); glVertex3f(1, -1, 1);
    glTexCoord2f(0, 1); glVertex3f(-1, -1, 1);
    glEnd();
}

void DrawTexturedTorus(GLuint tex, float r1 = 0.3f, float r2 = 1.0f, int sides = 30, int rings = 30) {
    glBindTexture(GL_TEXTURE_2D, tex);
    for (int i = 0;i < rings;i++) {
        float theta1 = (float)i / rings * 2 * M_PI;
        float theta2 = (float)(i + 1) / rings * 2 * M_PI;
        glBegin(GL_QUAD_STRIP);
        for (int j = 0;j <= sides;j++) {
            float phi = (float)j / sides * 2 * M_PI;
            for (float t = theta1;t <= theta2;t += theta2 - theta1) {
                float x = (r2 + r1 * cos(phi)) * cos(t);
                float y = r1 * sin(phi);
                float z = (r2 + r1 * cos(phi)) * sin(t);
                float u = t / (2 * M_PI);
                float v = phi / (2 * M_PI);
                glTexCoord2f(u, v);
                glVertex3f(x, y, z);
            }
        }
        glEnd();
    }
}

void DrawObjects() {
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    switch (currentObject) {
    case 0: {
        GLUquadric* q = gluNewQuadric();
        gluQuadricTexture(q, GL_TRUE);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        gluSphere(q, 1.0, 40, 40);
        gluDeleteQuadric(q);
        break;
    }
    case 1: DrawTexturedCube(textures[1]); break;
    case 2: {
        GLUquadric* q = gluNewQuadric();
        gluQuadricTexture(q, GL_TRUE);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        gluCylinder(q, 1.0, 1.0, 2.0, 40, 40);
        gluDisk(q, 0.0, 1.0, 40, 1);
        glTranslatef(0, 0, 2.0);
        gluDisk(q, 0.0, 1.0, 40, 1);
        glPopMatrix();
        gluDeleteQuadric(q);
        break;
    }
    case 3: {
        GLUquadric* q = gluNewQuadric();
        gluQuadricTexture(q, GL_TRUE);
        glBindTexture(GL_TEXTURE_2D, textures[3]);
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        gluCylinder(q, 1.0, 0.0, 2.0, 40, 40);
        gluDisk(q, 0.0, 1.0, 40, 1);
        glPopMatrix();
        gluDeleteQuadric(q);
        break;
    }
    case 4: DrawTexturedTorus(textures[4]); break;
    case 5: {
        glPushMatrix();
        glRotatef(cowRotation, 0, 1, 0);
        glCallList(CowList);
        glPopMatrix();
        break;
    }
    }

    glDisable(GL_TEXTURE_2D);
}

void Display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    float radX = camAngleX * M_PI / 180.0f;
    float radY = camAngleY * M_PI / 180.0f;
    float camX = 5 * cos(radY) * sin(radX);
    float camY = 5 * sin(radY);
    float camZ = 5 * cos(radY) * cos(radX);
    gluLookAt(camX, camY, camZ, 0, 0, 0, 0, 1, 0);

    float lightX = 3 * cos(lightAngle), lightZ = 3 * sin(lightAngle), lightY = 2.0f;
    GLfloat lightPos[] = { lightX,lightY,lightZ,1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    DrawObjects();
    glutSwapBuffers();
}

void InitGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);
    GLfloat lightColor[] = { 1.0f,1.0f,1.0f,1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor); glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH); glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    textures[0] = LoadBMP("venus.bmp");
    textures[1] = LoadBMP("jupiter.bmp");
    textures[2] = LoadBMP("neptune.bmp");
    textures[3] = LoadBMP("mars.bmp");
    textures[4] = LoadBMP("saturn.bmp");
    textures[5] = LoadBMP("pluto.bmp");

    InitCow();
}

void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (float)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Project 5: Cow 550");

    InitGL();

    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    glutIdleFunc(Idle);

#ifdef GLEW_VERSION
    glewInit();
#endif

    glutMainLoop();
    return 0;
}
