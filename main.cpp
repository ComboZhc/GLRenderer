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

#define GL_GLEXT_PROTOTYPES

#include "bitmap.hpp"
#include <GL/glut.h>
#include <GL/glext.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <cfloat>
#include <utility>
#include <algorithm>

using namespace std;

vector<GLfloat> vertex;
vector<GLfloat> tex;
vector<GLuint> vertexInd;
vector<GLuint> texInd;
vector<GLfloat> color;
GLuint texId;
GLuint fboId;
GLuint rbId;

BITMAPFILEHEADER imgHeader;
BITMAPINFOHEADER imgInfo;

BYTE* in_image;
BYTE* out_image;

GLenum err;
#define DEBUG() \
    err = glGetError();\
    if (err) {\
        printf("%s:%d Error 0x%x\n", __FILE__, __LINE__, err);\
        err = 0;\
    }

GLfloat light_ambient[]  = { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat light_position[] = { 0.0f, 2.0f, 2.0f, 1.0f };

int screen_width = 640;
int screen_height = 640;
char* src_file;
char* out_bmp;
char* in_bmp;
GLfloat eyeX = 0;
GLfloat eyeY = 0;
GLfloat eyeZ = 5;
GLfloat centerX = 0;
GLfloat centerY = 0;
GLfloat centerZ = 0;
GLfloat upX = 0;
GLfloat upY = 1;
GLfloat upZ = 0;
GLfloat colorR = 0;
GLfloat colorG = 0;
GLfloat colorB = 0;
int brightness = 0;
int contrast = 0;
float fovy = 45;
float tan_fovy_2 = tan(3.1415926 / 8);
bool auto_eye = false;

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
    if (auto_eye) {
        float maxL = 0;
        float eyeL = sqrt(eyeX * eyeX + eyeY * eyeY + eyeZ * eyeZ);
        for (int i = 0; i < vertex.size() / 3; ++i) {
            float aX = vertex[3 * i];
            float aY = vertex[3 * i + 1];
            float aZ = vertex[3 * i + 2];
            float aL = sqrt(aX * aX + aY * aY + aZ * aZ);
            float dot = (aX * eyeX + aY * eyeY + aZ * eyeZ);
            float cosA =  dot / aL / eyeL;
            float sinA = sqrt(1 - cosA * cosA);
            float h = sinA * aL;
            float t = h / tan_fovy_2;
            float l = t + dot / eyeL;
            if (l > maxL)
                maxL = l;
        }
        cout << maxL << endl;
        eyeX *= maxL / eyeL;
        eyeY *= maxL / eyeL;
        eyeZ *= maxL / eyeL;
    }

    vector<pair<GLuint, GLuint> > v;
    for (int i = 0; i < vertexInd.size(); ++i)
        v.push_back(make_pair(vertexInd[i], texInd[i]));
    sort(v.begin(), v.end());
    vector<pair<GLuint, GLuint> >::iterator it = unique(v.begin(), v.end());
    v.resize(it - v.begin());
    vector<GLfloat> new_vertex;
    vector<GLfloat> new_tex;
    vector<GLuint> new_ind;
    for (const pair<GLuint, GLuint>& p : v) {
        new_vertex.insert(new_vertex.end(), vertex.begin() + p.first * 3, vertex.begin() + p.first * 3 + 3);
        new_tex.insert(new_tex.end(), tex.begin() + p.second * 2, tex.begin() + p.second * 2 + 2);
    }
    for (int i = 0; i < vertexInd.size(); ++i)
        new_ind.push_back(lower_bound(v.begin(), v.end(), make_pair(vertexInd[i], texInd[i])) - v.begin());
    vertex.swap(new_vertex);
    tex.swap(new_tex);
    vertexInd.swap(new_ind);

    color.clear();
    for (int i = 0; i < vertexInd.size(); ++i) {
        color.push_back(0.0);
        color.push_back(0.0);
        color.push_back(0.0);
        color.push_back(1.0);
    }
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
    gluPerspective(fovy, ar, 0.1f, 100.0f);
    gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    glLoadIdentity();
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertex.data());
    glTexCoordPointer(2, GL_FLOAT, 0, tex.data());
    glColorPointer(4, GL_FLOAT, 0, color.data());
    glDrawElements(GL_TRIANGLES, vertexInd.size(), GL_UNSIGNED_INT, vertexInd.data());
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    DEBUG()
}

static void brightness_and_contrast() {
    float threshold = 127;
    contrast = contrast < -255 ? -255 : contrast > 254 ? 254 : contrast;
    BYTE values[256];
    for (int i = 0; i < 256; ++i) {
        int v;
        if (contrast >= 0)
            v = (i + brightness) + (i + brightness - threshold) * (1 / (1 - contrast / 255.0f) - 1) + 0.5f;
        else
            v = i + (i - threshold) * contrast / 255.0f + brightness + 0.5f;
        values[i] = v <= 0 ? 0 : v >= 255 ? 255 : v;
    }
    for (int i = 0; i < screen_width * screen_height * 3; ++i)
        out_image[i] = values[out_image[i]];
}

static void print() {
    out_image = (unsigned char*) malloc(screen_width * screen_height * 3);
    glReadPixels(0, 0, screen_width, screen_height, GL_BGR, GL_UNSIGNED_BYTE, out_image);
    brightness_and_contrast();
    imgHeader.bfSize = screen_width * screen_height * 3 + imgHeader.bfOffBits;
    imgInfo.biWidth = screen_width;
    imgInfo.biHeight = screen_height;
    saveBitmapFile(out_bmp, &imgHeader, &imgInfo, out_image);
    free(out_image);
    free(in_image);
}

static void display()
{
    draw();
    print();
    exit(0);
}

void shift(int* argc, char*** argv) {
    if (*argc == 0)
        exit(-1);
    --*argc;
    ++*argv;
}

void usage() {
    printf("GLRenderer -s src_obj -o out_bmp -i in_bmp [-w width] [-h height] [-l eyeX eyeY eyeZ centerX centerY centerZ upX upY upZ] [-c r g b] [-p x y z] [-t <contrast>] [-r <brightness>]\n");
    printf("-s, --src: source .obj file path\n");
    printf("-o, --out: out bmp file path\n");
    printf("-i, --in: in bmp file path\n");
    printf("-w, --width: width for screen, must be INT\n");
    printf("-h, --height: height for screen, must be INT\n");
    printf("-b, --bmp: texture bmp file path\n");
    printf("-l, --look: see gluLookAt, 9 FLOAT NUMBERS\n");
    printf("-p, --position: see glLight with GL_POSITION, 3 FLOAT NUMBERS\n");
    printf("-c, --color: background color\n");
    printf("-t, --contrast: contrast of the output, INT in [-255, 254]\n");
    printf("-r, --brightness: brightness of the output, INT in [-255, 254]\n");
    printf("-a, --auto: auto up position\n");
    exit(-1);
}

void parse_args(int argc, char** argv) {
    if (argc == 0)
        usage();
    while (argc > 0) {
        if (!strcmp(*argv, "-w") || !strcmp(*argv, "--width")) {
            shift(&argc, &argv);
            screen_width = atoi(*argv);
        } else if (!strcmp(*argv, "-h") || !strcmp(*argv, "--height")) {
            shift(&argc, &argv);
            screen_height = atoi(*argv);
        } else if (!strcmp(*argv, "-s") || !strcmp(*argv, "--src")) {
            shift(&argc, &argv);
            src_file = *argv;
        } else if (!strcmp(*argv, "-o") || !strcmp(*argv, "--out")) {
            shift(&argc, &argv);
            out_bmp = *argv;
        } else if (!strcmp(*argv, "-i") || !strcmp(*argv, "--in")) {
            shift(&argc, &argv);
            in_bmp = *argv;
        } else if (!strcmp(*argv, "-l") || !strcmp(*argv, "--look")) {
            shift(&argc, &argv);
            eyeX = atof(*argv);
            shift(&argc, &argv);
            eyeY = atof(*argv);
            shift(&argc, &argv);
            eyeZ = atof(*argv);
            shift(&argc, &argv);
            centerX = atof(*argv);
            shift(&argc, &argv);
            centerY = atof(*argv);
            shift(&argc, &argv);
            centerZ = atof(*argv);
            shift(&argc, &argv);
            upX = atof(*argv);
            shift(&argc, &argv);
            upY = atof(*argv);
            shift(&argc, &argv);
            upZ = atof(*argv);
        } else if (!strcmp(*argv, "-c") || !strcmp(*argv, "--color")) {
            shift(&argc, &argv);
            colorR = atof(*argv);
            shift(&argc, &argv);
            colorG = atof(*argv);
            shift(&argc, &argv);
            colorB = atof(*argv);
        } else if (!strcmp(*argv, "-p") || !strcmp(*argv, "--position")) {
            shift(&argc, &argv);
            light_position[0] = atof(*argv);
            shift(&argc, &argv);
            light_position[1] = atof(*argv);
            shift(&argc, &argv);
            light_position[2] = atof(*argv);
        } else if (!strcmp(*argv, "-r") || !strcmp(*argv, "--brightness")) {
            shift(&argc, &argv);
            brightness = atoi(*argv);
        } else if (!strcmp(*argv, "-t") || !strcmp(*argv, "--contrast")) {
            shift(&argc, &argv);
            contrast = atoi(*argv);
        } else if (!strcmp(*argv, "-a") || !strcmp(*argv, "--auto")) {
            auto_eye = true;
        } else
            usage();
        shift(&argc, &argv);
    }
}

/* Program entry point */

int main(int argc, char** argv)
{
    parse_args(argc - 1, &argv[1]);
    printf("parse done\n");
    int fakeArgc = 1;
    char fakeArgp[] = "GLRenderer";
    char *fakeArgv[] = { fakeArgp, NULL };
    glutInit(&fakeArgc, fakeArgv);
    printf("glut init\n");
    glutInitWindowSize(screen_width, screen_height);
    glutInitWindowPosition(0,0);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("GLRenderer");
    printf("glut created\n");
    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    load(src_file);
    reduce();
    in_image = loadBitmapFile(in_bmp, &imgHeader, &imgInfo);
    if (!in_image) {
        printf("image read failed\n");
        exit(-2);
    }
    printf("read completed\n");
    glClearColor(colorR, colorG, colorB, 0.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glEnable(GL_TEXTURE_2D);

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 imgInfo.biWidth,
                 imgInfo.biHeight,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 (GLvoid*)in_image);
    printf("loop\n");
    glutMainLoop();

    return EXIT_SUCCESS;
}
