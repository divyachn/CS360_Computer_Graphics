// g++ polygon.cpp -lGL -lGLU -lglut -lm

#include <GL/gl.h>
#include <GL/glut.h>

void display(void) {
    glClear (GL_COLOR_BUFFER_BIT);
    glColor3f (1.0, 1.0, 1.0);
    glBegin(GL_POLYGON);
        glVertex3f (3.0, 20.0, -2.0);
        glVertex3f (90.0, 20.0, -2.0);
        glVertex3f (90.0, 40.0, -2.0);
        glVertex3f (3.0, 40.0, -2.0);
    glEnd();
    glFlush ();
}

void init (void) {
    glClearColor (0.68, 0.71, 0.96, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // glOrtho(x_min, x_max, y_min, y_max, near_clipping_plane_distance, far_clipping_plane_distance)
    // z-coordinate can vary between -1.0 to -100.0
    glOrtho(0.0, 100.0, 0.0, 100.0, 1.0, 100.0);
}

void reshape (int w, int h) {
   glViewport (0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   glOrtho(0.0, 100.0, 0.0, 100.0, 1.0, 100.0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    // specifies the size, in pixels, of the window.
    glutInitWindowSize (1200,600);
    // specifies the screen location for the upper-left corner of the window - (x-LeftMargin, y-TopMargin)
    glutInitWindowPosition (100,50);
    glutCreateWindow ("Geometric Primitives");
    init ();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}
