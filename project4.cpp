#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctype.h>
#include <time.h>

#ifndef F_PI
#define F_PI        ((float)(M_PI))
#define F_2_PI      ((float)(2.f*F_PI))
#define F_PI_2      ((float)(F_PI/2.f))
#endif

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

class Keytimes {
private:
    static const int MAXKEYS = 100;
    float time[MAXKEYS];
    float value[MAXKEYS];
    int nKeys;
public:
    void Init() { nKeys = 0; }
    void AddTimeValue(float t, float v) {
        if (nKeys < MAXKEYS) { time[nKeys] = t; value[nKeys] = v; nKeys++; }
    }
    float GetValue(float t) {
        if (nKeys == 0) return 0.f;
        if (t <= time[0]) return value[0];
        if (t >= time[nKeys - 1]) return value[nKeys - 1];
        for (int i = 0; i < nKeys - 1; i++) {
            if (t >= time[i] && t <= time[i + 1]) {
                float f = (t - time[i]) / (time[i + 1] - time[i]);
                return value[i] * (1 - f) + value[i + 1] * f;
            }
        }
        return value[nKeys - 1];
    }
};

#define MSEC 10000

// Camera
Keytimes EyeX, EyeY, EyeZ;
Keytimes LookX, LookY, LookZ;
Keytimes UpX, UpY, UpZ;

Keytimes Obj1X, Obj1Y, Obj1Z;
Keytimes Obj1Rot, Obj1RotY, Obj1RotZ;
Keytimes Obj1Scale;

Keytimes Obj2X, Obj2Y, Obj2Z;
Keytimes Obj2Rot, Obj2RotY, Obj2RotZ;
Keytimes Obj2Scale;

GLuint DL1, DL2;

void OsuSphere(float radius, int slices, int stacks) {
    GLUquadric* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    gluSphere(q, radius, slices, stacks);
    gluDeleteQuadric(q);
}

void OsuCube(float size) {
    glutSolidCube(size);
}

void DrawAxes(float length) {
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    // X axis (red)
    glColor3f(1.f, 0.f, 0.f);
    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(length, 0.f, 0.f);
    // Y axis (green)
    glColor3f(0.f, 1.f, 0.f);
    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(0.f, length, 0.f);
    // Z axis (blue)
    glColor3f(0.f, 0.f, 1.f);
    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(0.f, 0.f, length);
    glEnd();
    glEnable(GL_LIGHTING);
}

void Resize(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void InitGraphics() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glClearColor(0.1f, 0.1f, 0.1f, 1.f);

    GLfloat ambient[] = { 0.2f,0.2f,0.2f,1.f };
    GLfloat diffuse[] = { 0.8f,0.8f,0.8f,1.f };
    GLfloat specular[] = { 1.f,1.f,1.f,1.f };
    GLfloat pos[] = { 5.f,5.f,5.f,1.f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);

    EyeX.Init(); EyeX.AddTimeValue(0.f, 0.f); EyeX.AddTimeValue(2.f, 3.f); EyeX.AddTimeValue(5.f, 0.f); EyeX.AddTimeValue(8.f, -3.f); EyeX.AddTimeValue(10.f, 0.f);
    EyeY.Init(); EyeY.AddTimeValue(0.f, 1.f); EyeY.AddTimeValue(2.f, 2.f); EyeY.AddTimeValue(5.f, 1.f); EyeY.AddTimeValue(8.f, 0.5f); EyeY.AddTimeValue(10.f, 1.f);
    EyeZ.Init(); EyeZ.AddTimeValue(0.f, 5.f); EyeZ.AddTimeValue(2.f, 4.f); EyeZ.AddTimeValue(5.f, 3.f); EyeZ.AddTimeValue(8.f, 4.f); EyeZ.AddTimeValue(10.f, 5.f);

    LookX.Init(); LookX.AddTimeValue(0.f, 0.f); LookX.AddTimeValue(2.f, 1.f); LookX.AddTimeValue(5.f, 0.f); LookX.AddTimeValue(8.f, -1.f); LookX.AddTimeValue(10.f, 0.f);
    LookY.Init(); LookY.AddTimeValue(0.f, 0.f); LookY.AddTimeValue(2.f, 0.5f); LookY.AddTimeValue(5.f, 0.f); LookY.AddTimeValue(8.f, -0.5f); LookY.AddTimeValue(10.f, 0.f);
    LookZ.Init(); LookZ.AddTimeValue(0.f, 0.f); LookZ.AddTimeValue(2.f, 1.f); LookZ.AddTimeValue(5.f, 0.f); LookZ.AddTimeValue(8.f, -1.f); LookZ.AddTimeValue(10.f, 0.f);

    UpX.Init(); UpX.AddTimeValue(0.f, 0.f); UpX.AddTimeValue(5.f, 0.1f); UpX.AddTimeValue(10.f, 0.f);
    UpY.Init(); UpY.AddTimeValue(0.f, 1.f); UpY.AddTimeValue(5.f, 0.95f); UpY.AddTimeValue(10.f, 1.f);
    UpZ.Init(); UpZ.AddTimeValue(0.f, 0.f); UpZ.AddTimeValue(5.f, 0.05f); UpZ.AddTimeValue(10.f, 0.f);

    Obj1X.Init(); Obj1X.AddTimeValue(0.f, -2.f); Obj1X.AddTimeValue(5.f, -0.5f); Obj1X.AddTimeValue(10.f, -2.f);
    Obj1Y.Init(); Obj1Y.AddTimeValue(0.f, 0.f); Obj1Y.AddTimeValue(5.f, 1.f); Obj1Y.AddTimeValue(10.f, 0.f);
    Obj1Z.Init(); Obj1Z.AddTimeValue(0.f, -1.f); Obj1Z.AddTimeValue(5.f, 0.f); Obj1Z.AddTimeValue(10.f, -1.f);

    Obj1Rot.Init(); Obj1Rot.AddTimeValue(0.f, 0.f); Obj1Rot.AddTimeValue(5.f, 180.f); Obj1Rot.AddTimeValue(10.f, 360.f);
    Obj1RotY.Init(); Obj1RotY.AddTimeValue(0.f, 0.f); Obj1RotY.AddTimeValue(5.f, 90.f); Obj1RotY.AddTimeValue(10.f, 180.f);
    Obj1RotZ.Init(); Obj1RotZ.AddTimeValue(0.f, 0.f); Obj1RotZ.AddTimeValue(5.f, 45.f); Obj1RotZ.AddTimeValue(10.f, 0.f);
    Obj1Scale.Init(); Obj1Scale.AddTimeValue(0.f, 1.f); Obj1Scale.AddTimeValue(5.f, 1.5f); Obj1Scale.AddTimeValue(10.f, 1.f);

    Obj2X.Init(); Obj2X.AddTimeValue(0.f, 2.f); Obj2X.AddTimeValue(5.f, 0.5f); Obj2X.AddTimeValue(10.f, 2.f);
    Obj2Y.Init(); Obj2Y.AddTimeValue(0.f, 0.f); Obj2Y.AddTimeValue(5.f, 1.5f); Obj2Y.AddTimeValue(10.f, 0.f);
    Obj2Z.Init(); Obj2Z.AddTimeValue(0.f, 1.f); Obj2Z.AddTimeValue(5.f, 0.f); Obj2Z.AddTimeValue(10.f, 1.f);

    Obj2Rot.Init(); Obj2Rot.AddTimeValue(0.f, 0.f); Obj2Rot.AddTimeValue(5.f, -180.f); Obj2Rot.AddTimeValue(10.f, 0.f);
    Obj2RotY.Init(); Obj2RotY.AddTimeValue(0.f, 0.f); Obj2RotY.AddTimeValue(5.f, -90.f); Obj2RotY.AddTimeValue(10.f, 0.f);
    Obj2RotZ.Init(); Obj2RotZ.AddTimeValue(0.f, 0.f); Obj2RotZ.AddTimeValue(5.f, -45.f); Obj2RotZ.AddTimeValue(10.f, 0.f);
    Obj2Scale.Init(); Obj2Scale.AddTimeValue(0.f, 1.f); Obj2Scale.AddTimeValue(5.f, 0.5f); Obj2Scale.AddTimeValue(10.f, 1.f);

    DL1 = glGenLists(1);
    glNewList(DL1, GL_COMPILE);
    OsuSphere(0.5f, 32, 32);
    glEndList();

    DL2 = glGenLists(1);
    glNewList(DL2, GL_COMPILE);
    OsuCube(1.f);
    glEndList();
}

void Animate() {
    glutPostRedisplay();
    glutTimerFunc(16, (void(*)(int))Animate, 0); // ~60fps
}

void Display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float t = (float)(glutGet(GLUT_ELAPSED_TIME) % MSEC) / 1000.f;

    gluLookAt(EyeX.GetValue(t), EyeY.GetValue(t), EyeZ.GetValue(t),
        LookX.GetValue(t), LookY.GetValue(t), LookZ.GetValue(t),
        UpX.GetValue(t), UpY.GetValue(t), UpZ.GetValue(t));

    DrawAxes(2.0f);

    GLfloat matRed[] = { 1.f, 0.f, 0.f, 1.f };
    GLfloat matSpec[] = { 1.f, 1.f, 1.f, 1.f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matRed);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);
    glMaterialf(GL_FRONT, GL_SHININESS, 50.f);

    glPushMatrix();
    glTranslatef(Obj1X.GetValue(t), Obj1Y.GetValue(t), Obj1Z.GetValue(t));
    glRotatef(Obj1Rot.GetValue(t), 1, 0, 0);
    glRotatef(Obj1RotY.GetValue(t), 0, 1, 0);
    glRotatef(Obj1RotZ.GetValue(t), 0, 0, 1);
    float s1 = Obj1Scale.GetValue(t);
    glScalef(s1, s1, s1);
    glCallList(DL1);
    glPopMatrix();

    GLfloat matBlue[] = { 0.f, 0.f, 1.f, 1.f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matBlue);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);
    glMaterialf(GL_FRONT, GL_SHININESS, 50.f);

    glPushMatrix();
    glTranslatef(Obj2X.GetValue(t), Obj2Y.GetValue(t), Obj2Z.GetValue(t));
    glRotatef(Obj2Rot.GetValue(t), 1, 0, 0);
    glRotatef(Obj2RotY.GetValue(t), 0, 1, 0);
    glRotatef(Obj2RotZ.GetValue(t), 0, 0, 1);
    float s2 = Obj2Scale.GetValue(t);
    glScalef(s2, s2, s2);
    glCallList(DL2);
    glPopMatrix();

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Keytime Animation Demo");

    InitGraphics();

    glutDisplayFunc(Display);
    glutReshapeFunc(Resize);
    Animate();

    glutMainLoop();
    return 0;
}
