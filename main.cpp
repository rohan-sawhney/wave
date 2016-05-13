#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <iomanip>
#include "Mesh.h"

int gridX = 600;
int gridY = 600;
int gridZ = 600;

const double fovy = 50.;
const double clipNear = .01;
const double clipFar = 1000.;
double x = 0.0, y = 0.0, z = 0.0;
double eyeX = 0.0, eyeY = 0.0, eyeZ = 2.5; // camera points initially along y-axis
double upX = 0.0, upY = 1.0, upZ = 0.0; // camera points initially along y-axis
double r = 2.5, theta = 0.0, phi = 0.0;

std::string path = "/Users/rohansawhney/Desktop/developer/C++/wave-mesh/kitten.obj";

Mesh mesh;
bool success = true;
bool stop = false;
double h = 0.001;
double a = 10;
double b = 0.005;
double min = 0.25;
double max = 1.0;

void initColors()
{
    for (VertexIter v = mesh.vertices.begin(); v != mesh.vertices.end(); v++) {
        v->computeNormal();
        v->prevColor = min;
        v->currColor = min;
    }
}

void setTitle()
{
    std::stringstream title;
    title << "Wave, h = " << std::setprecision(5) << h
          << "  a = " << std::setprecision(3) << a
          << "  b = " << std::setprecision(4) << b;
    glutSetWindowTitle(title.str().c_str());
}

void printInstructions()
{
    std::cerr << "' ': add random wave source\n"
              << "→/←: increase/decrease initial time step for wave\n"
              << "y/u: increase/decrease initial wave speed\n"
              << "i/o: increase/decrease initial wave damping\n"
              << "↑/↓: move in/out\n"
              << "w/s: move up/down\n"
              << "a/d: move left/right\n"
              << "p: pause\n"
              << "r: reload mesh\n"
              << "escape: exit program\n"
              << std::endl;
}

void init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
}

void draw()
{
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (EdgeCIter e = mesh.edges.begin(); e != mesh.edges.end(); e ++) {
        VertexIter v1 = e->he->vertex;
        VertexIter v2 = e->he->flip->vertex;
        
        const Eigen::Vector3d& a(v1->shiftedPosition);
        const Eigen::Vector3d& b(v2->shiftedPosition);
        
        double color = (v1->currColor + v2->currColor) * 0.5;
        if (color < min) color = min;
        glColor4f(0.0, 0.0, color, 1.0);
        
        glVertex3d(a.x(), a.y(), a.z());
        glVertex3d(b.x(), b.y(), b.z());
    }
    
    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    double aspect = (double)viewport[2] / (double)viewport[3];
    gluPerspective(fovy, aspect, clipNear, clipFar);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt(eyeX, eyeY, eyeZ, x, y, z, upX, upY, upZ);
    
    if (success) {
        draw();
    }

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x0, int y0)
{
    switch (key) {
        case 27 :
            exit(0);
        case 'r':
            success = mesh.read(path);
            if (success) {
                mesh.setup(h, a, b);
                initColors();
            }
            break;
        case ' ':
            mesh.vertices[rand() % mesh.vertices.size()].currColor = max;
            break;
        case 'a':
            x -= 0.03;
            break;
        case 'd':
            x += 0.03;
            break;
        case 'w':
            y += 0.03;
            break;
        case 's':
            y -= 0.03;
            break;
        case 'y':
            b -= 0.01;
            if (b < 0) b = 0;
            break;
        case 'u':
            b += 0.01;
            break;
        case 'i':
            a -= 0.1;
            if (a < 0) a = 0;
            break;
        case 'o':
            a += 0.1;
            break;
        case 'p':
            stop = !stop;
            break;
    }
    
    setTitle();
}

void special(int i, int x0, int y0)
{
    switch (i) {
        case GLUT_KEY_UP:
            z += 0.03;
            break;
        case GLUT_KEY_DOWN:
            z -= 0.03;
            break;
        case GLUT_KEY_LEFT:
            h -= 0.001;
            if (h < 0.001) h = 0.001;
            break;
        case GLUT_KEY_RIGHT:
            h += 0.001;
            break;
    }
    
    setTitle();
}

void mouse(int x, int y)
{
    // mouse point to angle conversion
    theta = (360.0 / gridY)*y*3.0;    // 3.0 rotations possible
   	phi = (360.0 / gridX)*x*3.0;
    
    // restrict the angles within 0~360 deg (optional)
   	if (theta > 360) theta = fmod((double)theta, 360.0);
   	if (phi > 360) phi = fmod((double)phi, 360.0);
    
    // spherical to cartesian conversion.
    // degrees to radians conversion factor 0.0174532
    eyeX = r * sin(theta*0.0174532) * sin(phi*0.0174532);
    eyeY = r * cos(theta*0.0174532);
   	eyeZ = r * sin(theta*0.0174532) * cos(phi*0.0174532);
    
    // reduce theta slightly to obtain another point on the same longitude line on the sphere.
    GLfloat dt = 1.0;
   	GLfloat eyeXtemp = r * sin(theta*0.0174532-dt) * sin(phi*0.0174532);
   	GLfloat eyeYtemp = r * cos(theta*0.0174532-dt);
   	GLfloat eyeZtemp = r * sin(theta*0.0174532-dt) * cos(phi*0.0174532);
    
    // connect these two points to obtain the camera's up vector.
   	upX = eyeXtemp - eyeX;
   	upY = eyeYtemp - eyeY;
   	upZ = eyeZtemp - eyeZ;
}

void idle()
{
    if (!stop) {
        mesh.computeWaveFlow(min, max);
        glutPostRedisplay();
    }
}

int main(int argc, char** argv) {
    
    success = mesh.read(path);
    if (success) {
        mesh.setup(h, a, b);
        initColors();
    }
    
    printInstructions();
    glutInitWindowSize(gridX, gridY);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInit(&argc, argv);
    glutCreateWindow("Wave, h = 0.001  a = 4  b = 0.01");
    init();
    
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutIdleFunc(idle);
    glutMotionFunc(mouse);
    glutMainLoop();
    
    return 0;
}
