/*
 * GLUT Shapes Demo
 *
 * Written by Nigel Stewart November 2003
 *
 * This program is test harness for the sphere, cone
 * and torus shapes in GLUT.
 *
 * Spinning wireframe and smooth shaded shapes are
 * displayed until the ESC or q key is pressed.  The
 * number of geometry stacks and slices can be adjusted
 * using the + and - keys.
 */

#include <windows.h>
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include "bitmap.hpp"

using namespace std;

vector<GLfloat> vertex;
vector<GLfloat> tex;
vector<GLuint> vertexInd;
vector<GLuint> texInd;
GLuint texId;
BYTE* image;
BITMAPFILEHEADER imgHeader;
BITMAPINFOHEADER imgInfo;
GLenum err;

#define DEBUG() \
    err = glGetError();\
    if (err) {\
        printf("%s:%d Error 0x%x\n", __FILE__, __LINE__, err);\
        err = 0;\
    }


/* Loader */

void load(const string& obj) {
    ifstream in(obj.c_str());
    cout << obj.c_str() << endl;
    string line;
    GLfloat f;
    GLuint ui;
    string first;
    while (in.good()) {
        in >> first;
        if (first == "v") {
            for (int i = 0; i < 3; ++i) {
                in >> f;
                vertex.push_back(f);
            }
        } else if (first == "vt") {
            for (int i = 0; i < 2; ++i) {
                in >> f;
                tex.push_back(f);
            }
        } else if (first == "f") {
            for (int i = 0; i < 3; ++i) {
                in >> ui;
                vertexInd.push_back(ui - 1);
                in.get();
                in >> ui;
                texInd.push_back(ui - 1);
            }
        }
        while ((in.peek() != '\n') && (in.get()));
        while ((in.peek() == '\n') && (in.get()));
    }
    cout << vertex.size() / 3 << endl;
    cout << tex.size() / 2 << endl;
    cout << vertexInd.size() / 3 << endl;
    cout << texInd.size() / 3 << endl;
}

void reduce() {
    vector<pair<GLuint, GLuint> > v;
    for (int i = 0; i < vertexInd.size(); ++i)
        v.push_back(make_pair(vertexInd[i], texInd[i]));
    sort(v.begin(), v.end());
    vector<pair<GLuint, GLuint> >::iterator it = unique(v.begin(), v.end());
    v.resize(it - v.begin());
    for (int i = 0; i < v.size(); ++i) {
        cout << v[i].first << ' ' << v[i].second << endl;
    }
    vector<GLfloat> new_vertex;
    vector<GLfloat> new_tex;
    vector<GLuint> new_ind;
    for (const pair<GLuint, GLuint>& p : v) {
        new_vertex.insert(new_vertex.end(), vertex.begin() + p.first * 3, vertex.begin() + p.first * 3 + 3);
        new_tex.insert(new_tex.end(), tex.begin() + p.second * 2, tex.begin() + p.second * 2 + 2);
    }
    for (int i = 0; i < vertexInd.size(); ++i)
        new_ind.push_back(lower_bound(v.begin(), v.end(), make_pair(vertexInd[i], texInd[i])) - v.begin());
    new_vertex.swap(vertex);
    new_tex.swap(tex);
    new_ind.swap(vertexInd);
}

void save(const string& obj) {
    ofstream out(obj.c_str());
    for (int i = 0; i < vertex.size() / 3; ++i)
        out << "v " << vertex[3 * i] << ' ' << vertex[3 * i + 1] << ' ' << vertex[3 * i + 2] << endl;
    for (int i = 0; i < tex.size() / 2; ++i)
        out << "vt " << tex[2 * i] << ' ' << tex[2 * i + 1] << endl;
    for (int i = 0; i < vertexInd.size() / 3; ++i)
        out << "f " << vertexInd[3 * i] + 1 << '/' << vertexInd[3 * i] + 1 << ' '
                    << vertexInd[3 * i + 1] + 1 << '/' << vertexInd[3 * i + 1] + 1 << ' '
                    << vertexInd[3 * i + 2] + 1 << '/' << vertexInd[3 * i + 2] + 1 << endl;
    out.close();
}

/* GLUT callback Handlers */

static void resize(int width, int height)
{
    const float ar = (float) width / (float) height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, ar, 0.1f, 100.0f);
    gluLookAt(0, 5, -5, 0, 0, 0, 0, 0, -1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertex.data());
    glTexCoordPointer(2, GL_FLOAT, 0, tex.data());
    glDrawElements(GL_TRIANGLES, vertexInd.size(), GL_UNSIGNED_INT, vertexInd.data());
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glutSwapBuffers();
}

static void idle(void)
{
    glutPostRedisplay();
}

const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

/* Program entry point */

int main(int argc, char *argv[])
{
    load("T:\\GLRenderer\\fengkan_10000.obj");
    reduce();
    save("T:\\GLRenderer\\fengkan_20000.obj");
    image = loadBitmapFile("T:\\GLRenderer\\fengkan_10000.bmp", &imgHeader, &imgInfo);

    if (image)
        cout << "GOT!" << endl;

    glutInit(&argc, argv);
    glutInitWindowSize(640,480);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("GLUT Shapes");
    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutIdleFunc(idle);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

//    glEnable(GL_LIGHT0);
//    glEnable(GL_NORMALIZE);
//    glEnable(GL_COLOR_MATERIAL);
//    glEnable(GL_LIGHTING);
//
//    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
//    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
//    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
//    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
//
//    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
//    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
//    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
//    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    glEnable(GL_TEXTURE_2D);

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 imgInfo.biWidth,
                 imgInfo.biHeight,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 (GLvoid*)image);
    DEBUG()
    //glutMainLoop();
    cout << "Hello World!" << endl;

    return EXIT_SUCCESS;
}
