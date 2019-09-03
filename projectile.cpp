// g++ projectile.cpp -lGL -lGLU -lglut -lm

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <string>
#include <cmath>
#include <sstream>
#include <iostream>

#define SPEED_ONE_UNIT 10
#define MAX_SPEED 100
#define MIN_SPEED 10
#define ANGLE_ONE_UNIT 5
#define MAX_ANGLE 90
#define MIN_ANGLE 10
#define LINE_LENGTH 50
#define UNIT_TIME 0.1
#define PI 3.1415926535
#define gravity 9.8

double degreeToRadian(float angle){
  double radian;
  radian = PI*angle/180;
  return radian;
}

GLUquadric* qobj;
float T=0;

class Ball{
private:
  int initial_x,initial_y,initial_z;
  float current_x,current_y,current_z;
  int speed;
  float direction;
  bool moving;
public:
  Ball(int x, int y, int z){
    initial_x = x;
    initial_y = y;
    initial_z = z;
    current_x=initial_x;
    current_y=initial_y;
    current_z=initial_z;
    speed = MIN_SPEED;
    direction = MIN_ANGLE;
    moving=false;
  }

  int getX() { return initial_x; }
  int getY() { return initial_y; }
  int getZ() { return initial_z; }

  int getSpeed() { return speed; }
  void setSpeed(int newSpeed) { speed=newSpeed; }

  float getDirection() { return direction; }
  void setDirection(float newDirection) { direction=newDirection; }

  void fire(){ moving=true; }

  void updatePosition(){
    // gravity is along the -y axis
    float ux = speed * cos( degreeToRadian(direction) );
    float uy = speed * sin( degreeToRadian(direction) );
    glLoadIdentity();
    glTranslatef(initial_x, initial_y, initial_z);
    current_x = ux * T;
    current_y = uy * T - ( gravity * T * T / 2);
    if(current_y<0){
      // std::cout<<"Ball hit the ground\n\n";
      moving=false;
      T=0;
      glTranslatef(current_x,0.0,0.0);
      glColor3f(1.0,0.0,0.0);
      gluSphere(qobj,2.0,2000,2000);
      return;
    }
    // std::cout<<current_x<<" , "<<current_y<<" , "<<current_z<<"\n";
    glTranslatef(current_x,current_y,0.0);
    glColor3f(1.0,0.0,0.0);
    gluSphere(qobj,2.0,2000,2000);
    T+=UNIT_TIME;
  }

  void renderFlyingBall(){
    if(moving==true){
      updatePosition();
    }
  }
};

Ball b(-25.0, 10.0, -50.0);

void normalKeys(unsigned char key, int x, int y){
  float d;
  int speed;
  switch(key) {
    case 32:
    // start the projectile motion when SPACE is pressed
      b.fire();
      break;
    case 27:
    // escape key -> exit
      exit(0);
      break;
    case 'a':
      // increase the angle
      d = b.getDirection();
      d+=ANGLE_ONE_UNIT;
      if(d>MAX_ANGLE) d=MAX_ANGLE;
      b.setDirection(d);
      break;
    case 's':
      // decrease the angle
      d = b.getDirection();
      d-=ANGLE_ONE_UNIT;
      if(d<MIN_ANGLE) d=MIN_ANGLE;
      b.setDirection(d);
      break;
    case 'z':
      // increase the speed
      speed = b.getSpeed();
      speed+=SPEED_ONE_UNIT;
      speed=std::min(MAX_SPEED,speed);
      b.setSpeed(speed);
      break;
    case 'x':
      // decrease the speed
      speed = b.getSpeed();
      speed-=SPEED_ONE_UNIT;
      speed=std::max(MIN_SPEED,speed);
      b.setSpeed(speed);
      break;
    default: break;
  }
  glutPostRedisplay();
}

std::string intToString(int n){

  if(n==0) return "0";

  std::string s = "";

  int temp=n;
  while(temp!=0){
    char ch = '0'+(temp%10);
    s = ch+s;
    temp=temp/10;
  }

  return s;
}

std::string floatToString(float n){
  std::ostringstream ss;
  ss << n;
  std::string s(ss.str());
  return s;
}

void writeText(float r, float g, float b, float x, float y, float z, std::string s){
  glLoadIdentity();
  glColor3f(r,g,b);
  glRasterPos3f(x,y,z);
  for (int i=0; i<s.size(); i++){
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
	}
}

void drawDirectionLine(){
  glLoadIdentity();
  glTranslatef(b.getX(), b.getY(), b.getZ());
  glColor3f(0.0,0.0,0.0); // black
  glBegin(GL_LINES);
    glVertex3f(0.0,0.0,0.0);
    glVertex3f( LINE_LENGTH*cos(degreeToRadian(b.getDirection())), LINE_LENGTH*sin(degreeToRadian(b.getDirection())), 0.0);
  glEnd();
}

void drawAxes(){
  glLoadIdentity();
  glTranslatef(b.getX(), b.getY(), b.getZ());
  glColor3f(0.0,0.0,0.0); // black
  glBegin(GL_LINES);
    glVertex3f(0.0,0.0,0.0);
    glVertex3f(1000.0,0.0,0.0);
    glVertex3f(0.0,500.0,0.0);
    glVertex3f(0.0,0.0,0.0);
  glEnd();
}

void drawSphere(){
  glLoadIdentity();
  glTranslatef(b.getX(), b.getY(), b.getZ());
  glColor3f(1.0, 0.0, 0.0); // red ball
  gluSphere(qobj,5.0,2000,2000);
}

void init() {
   glClearColor(0.78, 0.92, 0.95, 0.0);
   glEnable(GL_DEPTH_TEST);
   qobj = gluNewQuadric();
   gluQuadricNormals(qobj, GLU_SMOOTH);
   glShadeModel(GL_FLAT);
}

void display () {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);

    // viewing transformation
    gluLookAt(0.0,0.0,5.0,0.0,0.0,-5.0,0.0,1.0,0.0);

    // modeling transformation
    // drawSphere();
    drawAxes();
    drawDirectionLine();
    writeText(0.0,1.0,0.0,-80.0,-20.0,-50.0,"Speed: ");
    writeText(0.0,1.0,0.0,50.0,-20.0,-50.0,intToString(b.getSpeed()));
    writeText(0.0,0.0,1.0,120.0,-20.0,-50.0,"Direction: ");
    writeText(0.0,0.0,1.0,250.0,-20.0,-50.0,floatToString(b.getDirection()));
    b.renderFlyingBall();
    glFlush();
    glutSwapBuffers();
}

void reshape (int w, int h) {
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-100.0, 500.0, -10.0, 100.0, 20, 75);
   glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize (1200,600);
   glutInitWindowPosition (100, 50);
   glutCreateWindow ("Simple Projectile Motion");
   init();
   glutDisplayFunc(display);
   glutIdleFunc(display);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(normalKeys);
   glutMainLoop();
   return 0;
}
