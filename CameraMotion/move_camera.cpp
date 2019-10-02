// g++ move_camera.cpp -lGL -lGLU -lglut -lm

#include <GL/gl.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>

using namespace std;

#define PI 3.1415926535
#define XMAX 15.0
#define XMIN -15.0
#define YMAX 15.0
#define YMIN -15.0
#define ZMAX 15.0
#define ZMIN -15.0

struct vector{
  float x,y,z;
};

struct vector formVertex(float x, float y, float z){
  struct vector v;
  v.x=x;
  v.y=y;
  v.z=z;
  return v;
}

double degreeToRadian(float angle){
  // takes angle in degree, returns angle in radians
  double radian;
  radian = PI*angle/180;
  return radian;
}

class Camera{

private:
  struct vector pos;
  float angle_y, angle_z;
  float pos_increment;
  float angle_increment;

public:
  Camera(){
    // constructor
    pos.x=0.0; pos.y=0.0; pos.z=0.0;
    angle_y=90.0; angle_z=180.0;
    pos_increment = 1.0;
    angle_increment = 5.0;
  }
  float getAngleY() { return angle_y; }
  float getAngleZ() { return angle_z; }

  // functions to update camera_coordinates and up vector
  void updateX(float term){
    pos.x+=(term*pos_increment);
    if(pos.x>XMAX) pos.x = XMAX;
    if(pos.x<XMIN) pos.x = XMIN;
  }
  void updateY(float term){
    pos.y+=(term*pos_increment);
    if(pos.y>YMAX) pos.y = YMAX;
    if(pos.y<YMIN) pos.y = YMIN;
  }
  void updateZ(float term){
    pos.z+=(term*pos_increment);
    if(pos.z>ZMAX) pos.z = ZMAX;
    if(pos.z<ZMIN) pos.z = ZMIN;
  }
  void updateAngleY(float term){
    angle_y+=(term*angle_increment);
    if(angle_y<0.0) angle_y = 0-angle_y;
    else if(angle_y>180.0){
      angle_y = 180 - (angle_y-180);
    }
  }
  void updateAngleZ(float term){
    angle_z+=(term*angle_increment);
    if(angle_z<0.0)
      angle_z+=360.0;
    else if(angle_z>=360.0)
        angle_z-=360.0;
  }

  struct vector getCameraCoordinates(){
    return pos;
  }

  struct vector getLookAtVector(){
    struct vector look_at;
    look_at.x = 100*sin(degreeToRadian(angle_y)) * sin(degreeToRadian(angle_z));
    look_at.y = 100*cos(degreeToRadian(angle_y));
    look_at.z = 100*sin(degreeToRadian(angle_y)) * cos(degreeToRadian(angle_z));
    return look_at;
  }

};

Camera cam;

void enable (void) {
    glEnable (GL_DEPTH_TEST); //enable the depth testing
    // glEnable (GL_LIGHTING); //enable the lighting
    // glEnable (GL_LIGHT0); //enable LIGHT0, our Diffuse Light
    glShadeModel (GL_SMOOTH); //set the shader to smooth shader
}

void init (void) {
  // set the background color - white
  glClearColor(1.0, 1.0, 1.0, 1.0);
  enable();
}

void drawPolygon(struct vector vertices[], int n){
  glPushMatrix();
  glBegin(GL_POLYGON);
    for(int i=0;i<n;i++){
      glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
    }
  glEnd();
  glPopMatrix();
}

void draw_room(){
  // x constant, blue color
  glColor3f(0.0, 0.0, 1.0);
  struct vector face1[] = {formVertex(20.0, 20.0, 20.0), formVertex(20.0, 20.0, -20.0), formVertex(20.0, -20.0, -20.0), formVertex(20.0, -20.0, 20.0)};
  struct vector face2[] = {formVertex(-20.0, 20.0, 20.0), formVertex(-20.0, 20.0, -20.0), formVertex(-20.0, -20.0, -20.0), formVertex(-20.0, -20.0, 20.0)};
  drawPolygon(face1, 4);
  drawPolygon(face2, 4);
  // z constant, green color
  glColor3f(0.0, 1.0, 0.0);
  struct vector face3[] = {formVertex(20.0, 20.0, 20.0), formVertex(20.0, -20.0, 20.0), formVertex(-20.0, -20.0, 20.0), formVertex(-20.0, 20.0, 20.0)};
  struct vector face4[] = {formVertex(20.0, 20.0, -20.0), formVertex(20.0, -20.0, -20.0), formVertex(-20.0, -20.0, -20.0), formVertex(-20.0, 20.0, -20.0)};
  drawPolygon(face3, 4);
  drawPolygon(face4, 4);
  // y constant, red color
  glColor3f(1.0, 0.0, 0.0);
  struct vector face5[] = {formVertex(20.0, 20.0, 20.0), formVertex(20.0, 20.0, -20.0), formVertex(-20.0, 20.0, -20.0), formVertex(-20.0, 20.0, 20.0)};
  struct vector face6[] = {formVertex(20.0, -20.0, 20.0), formVertex(20.0, -20.0, -20.0), formVertex(-20.0, -20.0, -20.0), formVertex(-20.0, -20.0, 20.0)};
  drawPolygon(face5, 4);
  drawPolygon(face6, 4);
}

void specialKeys(int key, int x, int y){

  switch (key) {

    case GLUT_KEY_LEFT:
      cam.updateAngleY(-1);
      break;

    case GLUT_KEY_RIGHT:
      cam.updateAngleY(1);
      break;

    case GLUT_KEY_UP:
      cam.updateAngleZ(1);
      break;

    case GLUT_KEY_DOWN:
      cam.updateAngleZ(-1);
      break;

    default: break;
  }
  glutPostRedisplay();
}

void normalKeys (unsigned char key, int x, int y) {
  if(key==27) exit(0);  //escape key
  switch(key){
    case 'a': // move along +ve X-axis
      cam.updateX(1);
      break;
    case 'b': // move along -ve X-axis
      cam.updateX(-1);
      break;
    case 'c': // move along +ve Y-axis
      cam.updateY(1);
      break;
    case 'd': // move along -ve Y-axis
      cam.updateY(-1);
      break;
    case 'e': // move along +ve Z-axis
      cam.updateZ(1);
      break;
    case 'f': // move along -ve Z-axis
      cam.updateZ(-1);
      break;
    default:
      break;
  }
  glutPostRedisplay();
}

void display (void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    struct vector camera_coordinates = cam.getCameraCoordinates();
    struct vector look_at = cam.getLookAtVector();
    cout<<"Camera Coordinates: ";
    cout<<camera_coordinates.x<<" "<<camera_coordinates.y<<" "<<camera_coordinates.z<<"\n";
    cout<<"Angle_z: "<<cam.getAngleZ()<<" Angle_y: "<<cam.getAngleY()<<"\n";
    cout<<"Look at vector: ";
    cout<<look_at.x<<" "<<look_at.y<<" "<<look_at.z<<"\n\n";
    gluLookAt(camera_coordinates.x, camera_coordinates.y, camera_coordinates.z,
              look_at.x, look_at.y, look_at.z,
              0.0, 1.0, 0.0);
    draw_room();
    glFlush();
    glutSwapBuffers();
}

void reshape (int w, int h) {
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-30.0, 30.0, 30.0, -30.0, 5, 30.0);
   glMatrixMode(GL_MODELVIEW);
}

int main (int argc, char **argv) {
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize (1000,600);
    glutInitWindowPosition (120, 50);
    glutCreateWindow ("Move Camera");
    init ();
    glutDisplayFunc (display);
    // glutIdleFunc (display);
    glutReshapeFunc (reshape);
    glutKeyboardFunc(normalKeys);
    glutSpecialFunc(specialKeys);
    glutMainLoop ();
    return 0;
}
