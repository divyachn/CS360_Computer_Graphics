// g++ game.cpp -lGL -lGLU -lglut -lm

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <string>
#include <cmath>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

#define ZDEPTH 50.0
#define gravity 9.8
#define PI 3.1415926535
#define UNIT_TIME 0.05
#define MAX_SPEED 100
#define MIN_SPEED 10
#define SPEED_ONE_UNIT 5
#define NOZZLE_SPIN 5
#define MAX_ANGLE 85
#define MIN_ANGLE 0
#define NOZZLE_HEIGHT 13.0

double degreeToRadian(float angle){
  // takes angle in degree, returns angle in radians
  double radian;
  radian = PI*angle/180;
  return radian;
}

std::string intToString(int n){
  // takes an integer, and returns the equivalent string
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

GLUquadric* qobj;
int turn=0; // keeps track of the turn of player
float T; // time t for projectile motion
bool gameBegin=true, gameOver=false;
/* flag_for_score can have three discrete values:
  { 0: bullet hit the ground, no tank awarded any score,
    1: increment score of tank1 by 1,
    2: increment score of tank2 by 1 }
*/
int flag_for_score=0, reset=0;
float time_of_flight;

/* opponent tank - data - globally stored */
float target_x, target_y, target_z = ZDEPTH+0.5+4;;          // coordinates of the center of the base region of the opponent tank
float tank_width=20.0 , tank_height=10.0;    // dimensions of the base of the tank, constant for both the players

/* specify the camera coordinates */
float xpos = 0, ypos = 0, zpos = 0, xrot = 0, yrot = 0, angle=0.0;
float cRadius = 10.0f; // our radius distance from our character
float lastx, lasty;

class SquareRegion{
private:
  float center_x, center_y, width, height;
public:
  SquareRegion(float center_x, float center_y, float width, float height){
    this->center_x = center_x;
    this->center_y = center_y;
    this->width = width;
    this->height = height;
  }
  float getWidth() {return width;}
  float getHeight() {return height;}
  float getX_min() {return center_x-(width/2);}
  float getX_max() {return center_x+(width/2);}
  float getY_min() {return center_y-(height/2);}
  float getY_max() {return center_y+(height/2);}
  void drawSquare() {
    glPushMatrix();
    glColor3f(0.92, 0.76, 0.81);
    glTranslatef(center_x,center_y,0-ZDEPTH+0.5);
    glBegin(GL_POLYGON);
      glVertex3f(width/2,0-height/2,0.0);
      glVertex3f(width/2,height/2,0.0);
      glVertex3f(0-width/2,height/2,0.0);
      glVertex3f(0-width/2,0-height/2,0.0);
    glEnd();
    glPopMatrix();
  }
};

SquareRegion square1(-50.0,0.0,40.0,40.0);
SquareRegion square2(50.0,0.0,40.0,40.0);

class Missile {

private:

  float current_x, current_y, current_z;  // coordinates of the bullet when it flies
  float initial_x, initial_y, initial_z;  // it is the position of the nozzle (of the tank)
  float speed_x, speed_y, speed_z;        // In the rotated frame, there is no motion along the y-axis, speed_y=0
  float direction;                        // direction - angle the nozzle makes with X-axis in the XY plane
  bool moving;                            // detects whether the bullet is flying or not
  int tankNumber;                         // this nozzle belongs to which tank

public:

  void setTankNum(int n) { tankNumber = n; moving=false; }

  void setPosition(float px, float py, float pz, int inclination) {
    /* initial_ : These are coordinates wrt the world (absolute coordinates)
      direction is the angle the nozzle makes with the +ve X-axis for both the tanks, inclination is the angle made be the nozzle with the XY plane
    */
    initial_x = px + NOZZLE_HEIGHT*cos(degreeToRadian(direction))*cos(degreeToRadian(inclination));
    initial_y = py + NOZZLE_HEIGHT*sin(degreeToRadian(direction));
    initial_z = pz + NOZZLE_HEIGHT*cos(degreeToRadian(direction))*sin(degreeToRadian(inclination));
    current_x = initial_x;
    current_y = initial_y;
    current_z = initial_z;
  }

  void setSpeed(int u, int inclination){
    speed_x = u * cos(degreeToRadian(inclination));
    speed_y = 0;
    speed_z = u * sin(degreeToRadian(inclination));
    // time_of_flight = (speed_z/G) + sqrt(2*(NOZZLE_HEIGHT*sin(inclination)+initial_z)/G);
    time_of_flight = 1000;
  }

  void setDirection(float d){ direction = d; }

  bool isBulletMoving() { return moving; }

  void updateMoving(bool b) { moving = b; }

  void updateBulletPosition(){
    /* obtain the new position of the bullet after assuming the transformed coordinates */
    float new_z = (speed_z * T) - ( gravity * T * T / 2); // gravity is along the -z axis
    float new_y = 0;  // no motion along the y-axis
    float new_x = speed_x * T; // x-axis is the horizontal direction
    // rotate back by -direction along the negative axis
    current_x = new_x * cos(degreeToRadian(direction));
    current_y = 0 - new_x * sin(degreeToRadian(direction));
    current_z = new_z;
    // translate back the origin
    current_x += initial_x;
    current_y += initial_y;
    current_z += initial_z;

    std::cout<<"This bullet is fired by TANK - "<<tankNumber<<"\n\n";
    std::cout<<initial_x<<" , "<<initial_y<<" , "<<initial_z;
    std::cout<<" @ T = "<<T<<" -> ";
    std::cout<<current_x<<" , "<<current_y<<" , "<<current_z<<"\n";

    if( (current_x>target_x-tank_width/2) && (current_x<target_x+tank_width/2) && (current_y>target_y-tank_height/2) && (current_y<target_y+tank_height/2) ){
      // check if the bullet is inside the base region of the other tank
      if(current_z <= target_z){
        // check if the bullet has entered in the vertical height region or not
        std::cout<<"Bullet hit the other tank\n\n";
        reset=1;
        moving = false;
        flag_for_score = tankNumber;
        glPushMatrix();
          glTranslatef(current_x, current_y, target_z);
          glColor3f(1.0,0.0,0.0);
          gluSphere(qobj,2.0,2000,2000);
        glPopMatrix();
        return;
      }
    }
    if(current_z<= 0-ZDEPTH+0.5){
      /* check if the bullet has hit the ground or not */
      std::cout<<"Bullet hit the GROUND\n\n";
      reset=1;
      moving=false;
      glPushMatrix();
        glTranslatef(current_x, current_y, 0-ZDEPTH+0.5);
        glColor3f(1.0,0.0,0.0);
        gluSphere(qobj,2.0,2000,2000);
      glPopMatrix();
      return;
    }
    glPushMatrix();
      glTranslatef(current_x,current_y,current_z);
      glColor3f(1.0,0.0,0.0);
      gluSphere(qobj,2.0,2000,2000);
    glPopMatrix();
    T+=UNIT_TIME;
  }

  void renderBullet(){
    if(moving==true) { updateBulletPosition(); }
  }

};

class Tank{

private:

  float x,y,z,r,g,b;  // base and color of tank
  int score;  // score for this tank
  float base_x, base_y; // dimensions of the base
  int direction, inclination, bulletSpeed; // direction:- angle from the X-axis in XY plane and inclination:- angle from the ground
  Missile m;  // associated bullet

public:

  Tank(float x, float y, float z, float r, float g, float b, int n){
    // set the default position and color of the tank
    this->x = x;
    this->y = y;
    this->z = z;
    this->r = r;
    this->g = g;
    this->b = b;
    // dimensions of the base are set here (default, fixed)
    base_x = tank_width;
    base_y = tank_height;
    score = 0;  // set the score of the tank
    // nozzle by default along the x-axis, for tankB 180 maps to 0 degree angle
    if(x<0) direction = 0;
    else direction = 180-0;
    inclination = 0;
    bulletSpeed = MIN_SPEED;
    m.setTankNum(n);
    m.setSpeed(bulletSpeed,inclination);
    m.setDirection(direction);
    m.setPosition(x,y,z+4,inclination);
  }

  float getX() { return x; }
  float getY() { return y; }
  float getZ() { return z; }

  float getBase_X() { return base_x; }
  float getBase_Y() { return base_y; }

  int getDirection() { return direction; }
  void setDirection(int d) { direction=d; m.setDirection(d); m.setPosition(x,y,z+4,inclination); }

  int getInclination() { return inclination; }
  void setInclination(int i) { inclination=i; m.setSpeed(bulletSpeed, i); m.setPosition(x,y,z+4,inclination); }

  int getBulletSpeed() { return bulletSpeed; }
  void setBulletSpeed(int b) { bulletSpeed=b; m.setSpeed(bulletSpeed, inclination); }

  char getScore() { return '0'+score; }
  void updateScore(){
    score++;
    if(score==5) gameOver=true; // this player has won the game
  }

  void fire() { m.updateMoving(true); }
  bool isBulletMoving(){ return m.isBulletMoving(); }
  void renderBullet(){ m.renderBullet(); }

  void updateX(float addx, float width, float x_min, float x_max){
    if(addx<0){
      // left key is pressed
      if(x-(width/2)+addx<x_min) x=x_min+(width/2);
      else x+=addx;
    } else{
      // right key is pressed
      if(x+(width/2)+1>x_max) x=x_max-(width/2);
      else x+=addx;
    }
    m.setPosition(x,y,z+4,inclination);
  }

  void updateY(float addy, float height, float y_min, float y_max){
    if(addy<0){
      // left key is pressed
      if(y-(height/2)+addy<y_min) y=y_min+5.0;
      else y+=addy;
    } else{
      // right key is pressed
      if(y+(height/2)+1>y_max) y=y_max-(height/2);
      else y+=addy;
    }
    m.setPosition(x,y,z+4,inclination);
  }

  void drawTank(){
    glColor3f(r,g,b);
    /* translate to the position of the tank ->
       draw the base cuboid ->
       again translate to the center of the upper face of the base cuboid ->
       draw the top cuboid ->
       translate to the center of the upper face of the upper cuboid ->
       draw the nozzle(it is a cylinder and it's axis can be controlled by the user)
    */
    glPushMatrix();
    glTranslatef(x,y,z);
    glScalef(base_x,base_y,4.0);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x,y,z+2.0); // since the height along z-axis is 4 and it is origin centered therefore the increment along +ve and -ve axis is 4/2.
    glScalef(10.0,6.0,4.0);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.33, 0.25, 0.39);
    glTranslatef(x,y,z+4);  // come on top of the upper cuboid
    // align x-axis along the direction set by the user/default
    glRotatef(direction, 0.0, 0.0, 1.0);
    // elevate the nozzle as set by the user/default, don't forget 90-inclination!
    glRotatef(90-inclination, 0.0, 1.0, 0.0);
    gluCylinder(qobj,2.0,1.5,NOZZLE_HEIGHT,600,600);
    glPopMatrix();
  }

};

Tank tank1(0-50.0, 0.0, 0-ZDEPTH+0.5, 0.05, 0.08, 0.55, 1);
Tank tank2(50.0, 0.0, 0-ZDEPTH+0.5, 0.05, 0.91, 0.15, 2);

void drawSphere(float x,float y, float z){
  glPushMatrix();
  glTranslatef(x,y,z);
  glColor3f(1.0, 1.0, 1.0);
  gluSphere(qobj,3.0,2000,2000);
  glPopMatrix();
}

void redirect(int button, int state, int x, int y){
  gameBegin=false;
  glutPostRedisplay();
}

void specialKeys(int key, int x, int y){

  // if it is the turn of player A/B and bullet has been fired but not hit the ground so far => disable keyboard function
  if(turn%2==0 && tank1.isBulletMoving()==true) return;
  if(turn%2==1 && tank2.isBulletMoving()==true) return;

  if(turn%2==1){ return; }

  switch (key) {

    case GLUT_KEY_LEFT:
      tank1.updateX(-1.0,tank1.getBase_X(),square1.getX_min(),square1.getX_max());
      break;

    case GLUT_KEY_RIGHT:
      tank1.updateX(1.0,tank1.getBase_X(),square1.getX_min(),square1.getX_max());
      break;

    case GLUT_KEY_UP:
      tank1.updateY(1.0,tank1.getBase_Y(),square1.getY_min(),square1.getY_max());
      break;

    case GLUT_KEY_DOWN:
      tank1.updateY(-1.0,tank1.getBase_Y(),square1.getY_min(),square1.getY_max());
      break;

    default: break;
  }
  glutPostRedisplay();
}

void normalKeys( unsigned char key, int x, int y ) {

  if(key==27) exit(0);  //escape key

  if(gameBegin==true || gameOver==true) return;

  // handle camera transformations
  if (key=='z') {
    xrot += 1;
    if (xrot >360) xrot -= 360;
    return;
  }
  if (key=='x') {
    xrot -= 1;
    if (xrot < -360) xrot += 360;
    return;
  }
  if (key=='c') {
    float xrotrad, yrotrad;
    yrotrad = (yrot / 180 * 3.141592654f);
    xrotrad = (xrot / 180 * 3.141592654f);
    xpos += float(sin(yrotrad));
    zpos -= float(cos(yrotrad));
    ypos -= float(sin(xrotrad));
    return;
  }
  if (key=='v') {
    float xrotrad, yrotrad;
    yrotrad = (yrot / 180 * 3.141592654f);
    xrotrad = (xrot / 180 * 3.141592654f);
    xpos -= float(sin(yrotrad));
    zpos += float(cos(yrotrad));
    ypos += float(sin(xrotrad));
    return;
  }
  if (key=='b') {
    float yrotrad;
    yrotrad = (yrot / 180 * 3.141592654f);
    xpos += float(cos(yrotrad)) * 0.2;
    zpos += float(sin(yrotrad)) * 0.2;
    return;
  }
  if (key=='n') {
    float yrotrad;
    yrotrad = (yrot / 180 * 3.141592654f);
    xpos -= float(cos(yrotrad)) * 0.2;
    zpos -= float(sin(yrotrad)) * 0.2;
    return;
  }

  // if it is the turn of player A/B and bullet has been fired but not hit the ground so far => disable keyboard function
  if(turn%2==0 && tank1.isBulletMoving()==true) return;
  if(turn%2==1 && tank2.isBulletMoving()==true) return;

  // set the bullet Speed: -> t+, y-
  if(key=='t'){
    int bulletSpeed;
    if(turn%2==0) bulletSpeed = tank1.getBulletSpeed();
    else bulletSpeed = tank2.getBulletSpeed();
    bulletSpeed = std::min(MAX_SPEED, bulletSpeed+SPEED_ONE_UNIT);
    if(turn%2==0) tank1.setBulletSpeed(bulletSpeed);
    else tank2.setBulletSpeed(bulletSpeed);
    return;
  }
  if(key=='y'){
    int bulletSpeed;
    if(turn%2==0) bulletSpeed = tank1.getBulletSpeed();
    else bulletSpeed = tank2.getBulletSpeed();
    bulletSpeed = std::max(MIN_SPEED, bulletSpeed-SPEED_ONE_UNIT);
    if(turn%2==0) tank1.setBulletSpeed(bulletSpeed);
    else tank2.setBulletSpeed(bulletSpeed);
    return;
  }

  switch (key) {

    // Set position of the tank2
    case 'a':
      if(turn%2==0) return;
      tank2.updateX(-1.0,tank2.getBase_X(),square2.getX_min(),square2.getX_max());
      break;
    case 's':
      if(turn%2==0) return;
      tank2.updateX(1.0,tank2.getBase_X(),square2.getX_min(),square2.getX_max());
      break;
    case 'd':
      if(turn%2==0) return;
      tank2.updateY(1.0,tank2.getBase_Y(),square2.getY_min(),square2.getY_max());
      break;
    case 'f':
      if(turn%2==0) return;
      tank2.updateY(-1.0,tank2.getBase_Y(),square2.getY_min(),square2.getY_max());
      break;

    // fire the bullet
    case 32:
      // player A fired the bullet -> bullet goes from tank1 to tank2
      if(turn%2==0) {
        target_x = tank2.getX();
        target_y = tank2.getY();
        tank1.fire();
      }
      // player B fired the bullet -> bullet goes from tank2 to tank1
      else {
        target_x = tank1.getX();
        target_y = tank1.getY();
        tank2.fire();
      }
      break;

    // set Nozzle Direction -> q+, w-
    case 'q':
      if(turn%2==0){
        int direction = tank1.getDirection();;
        direction+=NOZZLE_SPIN;
        direction=direction%360;
        tank1.setDirection(direction);
      } else{
        int direction = tank2.getDirection();
        direction-=NOZZLE_SPIN;
        if(direction<0) direction+=360;
        direction=direction%360;
        tank2.setDirection(direction);
      }
      break;
    case 'w':
      if(turn%2==0){
        int direction = tank1.getDirection();
        direction-=NOZZLE_SPIN;
        if(direction<0) direction+=360;
        direction=direction%360;
        tank1.setDirection(direction);
      } else{
        int direction = tank2.getDirection();;
        direction+=NOZZLE_SPIN;
        direction=direction%360;
        tank2.setDirection(direction);
      }
      break;

    // set NOZZLE Inclination -> e+, r-
    case 'e':
      if(turn%2==0){
        int inclination = tank1.getInclination();
        inclination = std::min(90-NOZZLE_SPIN, inclination+NOZZLE_SPIN);
        tank1.setInclination(inclination);
      }else{
        int inclination = tank2.getInclination();
        inclination = std::min(90-NOZZLE_SPIN, inclination+NOZZLE_SPIN);
        tank2.setInclination(inclination);
      }
      break;
    case 'r':
      if(turn%2==0){
        int inclination = tank1.getInclination();
        inclination = std::max(0, inclination-NOZZLE_SPIN);
        tank1.setInclination(inclination);
      }else{
        int inclination = tank2.getInclination();
        inclination = std::max(0, inclination-NOZZLE_SPIN);
        tank2.setInclination(inclination);
      }
      break;

    default: break;
  }
  glutPostRedisplay();
}

void mouseMovement(int x, int y) {
  int diffx=x-lastx; //check the difference between the current x and the last x position
  int diffy=y-lasty; //check the difference between the current y and the last y position
  lastx=x; //set lastx to the current x position
  lasty=y; //set lasty to the current y position
  xrot += (float) diffy; //set the xrot to xrot with the addition of the difference in the y position
  yrot += (float) diffx;    //set the xrot to yrot with the addition of the difference in the x position
}

void init(void) {
  // set the background color - bluish(sky)
   glClearColor(0.78, 0.92, 0.95, 0.0);
   glEnable(GL_DEPTH_TEST);
   qobj = gluNewQuadric();
   gluQuadricNormals(qobj, GLU_SMOOTH);
   glShadeModel(GL_FLAT);
}

void writeText(float r, float g, float b, float x, float y, float z, std::string s){

  if(gameBegin==true || gameOver==true) glLoadIdentity();
  else glPushMatrix();

  glColor3f(r,g,b);
  glRasterPos3f(x,y,z);
  for (int i=0; i<s.size(); i++){
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
	}

  if(gameBegin==false && gameOver==false) glPopMatrix();
}

void gameBeginScreen(){
  glLoadIdentity();
  glTranslatef(0.0,0.0,0-ZDEPTH-10);
  writeText(0.0, 0.0, 0.0, 0-150.0, 100.0, 0-ZDEPTH-10, "CS360 - ASSIGNMENT1 - Tank GAME");
  writeText(1.0, 0.0, 0.0, 0-140.0, 80.0, 0-ZDEPTH-10, "Instructions: ");
  writeText(0.05, 0.64, 0.02, 0-120.0, 60.0, 0-ZDEPTH-10, "Player A uses [arrow] keys to navigate");
  writeText(0.0, 0.0, 1.0, 0-120.0, 40.0, 0-ZDEPTH-10, "Player B uses [asdf] keys to navigate");
  writeText(0.0, 0.0, 0.0, 0-120.0, 20.0, 0-ZDEPTH-10, "direction(q+ w-), inclination(e+ r-), bulletSpeed(t+ y-)");
  writeText(0.0, 0.0, 0.0, 0-120.0, 0.0, 0-ZDEPTH-10, "Use SPACE BAR to fire");
  writeText(0.0, 0.0, 0.0, 0-120.0, 0-20.0, 0-ZDEPTH-10, "Use [zxcvbn]/mouse to move the camera");
  writeText(0.0, 0.0, 0.0, 0-150.0, 0-50.0, 0-ZDEPTH-10, "Click to Continue...");
}

void gameOverScreen(){
  // Game is over: One of the player has scored the Perfect FIVE -> Display the game over screen
  glLoadIdentity();
  std::string winner, loser;
  if(tank1.getScore()=='0'+5){
    winner = "A won the game. :)";
    loser = "B better luck next time!";
  } else{
    winner = "B won the game. :)";
    loser = "A better luck next time!";
  }
  writeText(0.0, 0.0, 0.0, 0-30.0, 30.0, 0-ZDEPTH, "GAME OVER");
  writeText(0.64, 0.02, 0.63, 0-40.0, 0.0, 0-ZDEPTH, winner);
  writeText(0.96, 0.88, 0.38, 0-40.0, 0-20.0, 0-ZDEPTH, loser);
  writeText(0.0, 0.0, 0.0, 0-40.0, 0-50.0, 0-ZDEPTH, "PRESS [esc] to quit.");
}

void drawBase(){
  glPushMatrix();
  glTranslatef(0.0,0.0,0-ZDEPTH);
  glScalef(200.0, 150.0, 1.0);
  glColor3f(0.33, 0.13, 0.01);
  glutSolidCube (1.0);
  glPopMatrix();
}

void display(void) {
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // viewing transformation
   glLoadIdentity ();
  //  gluLookAt(camera_x, 1.0f, camera_z, camera_x+losx, 1.0f,  camera_z+losz, 0.0f, 1.0f,  0.0f);
   glTranslatef(0.0f, 0.0f, -cRadius);
   glRotatef(xrot,1.0,0.0,0.0);
   // modeling transformation
   if(gameBegin==true) gameBeginScreen();
   else if(gameOver==false){
    drawBase();
    square1.drawSquare();
    square2.drawSquare();
    tank1.drawTank();
    tank2.drawTank();
    if(flag_for_score==1) tank1.updateScore();
    else if(flag_for_score==2) tank2.updateScore();
    if(reset==1){
      // disable the current player and allow the other player to move
      T=0;
      flag_for_score=0; // switch to the default state - no player is being awarded any score
      turn++;
      reset=0;
    }
    std::string score1(1, tank1.getScore());
    std::string score2(1, tank2.getScore());
    writeText(0.89, 0.95, 0.17, -50.0, -60.0, 0-ZDEPTH+0.5, score1);
    writeText(0.89, 0.95, 0.17, -10.0, -60.0, 0-ZDEPTH+0.5, "Score");
    writeText(0.89, 0.95, 0.17, 50.0, -60.0, 0-ZDEPTH+0.5, score2);

    writeText(0.89, 0.95, 0.17, -50.0, 40.0, 0-ZDEPTH+0.5, intToString(tank1.getBulletSpeed()));
    writeText(0.89, 0.95, 0.17, -10.0, 40.0, 0-ZDEPTH+0.5, "Speed");
    writeText(0.89, 0.95, 0.17, 50.0, 40.0, 0-ZDEPTH+0.5, intToString(tank2.getBulletSpeed()));

    writeText(0.89, 0.95, 0.17, -50.0, 60.0, 0-ZDEPTH+0.5, "A");
    writeText(0.89, 0.95, 0.17, -10.0, 60.0, 0-ZDEPTH+0.5, "Player");
    writeText(0.89, 0.95, 0.17, 50.0, 60.0, 0-ZDEPTH+0.5, "B");
    tank1.renderBullet();
    tank2.renderBullet();
    glRotatef(yrot,0.0,1.0,0.0);  //rotate our camera on they-axis (up and down)
    glTranslated(-xpos,0.0f,-zpos); //translate the screen to the position of our camera
    angle++;
  } else{
    gameOverScreen();
  }
   glFlush();
   glutSwapBuffers();
}

void reshape (int w, int h) {
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-100.0, 100.0, -50.0, 50.0, 20, 75.0);
   glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize (1200,600);
   glutInitWindowPosition (100, 50);
   glutCreateWindow ("CS360: Assignment1 - Tank Game");
   init();
   glutDisplayFunc(display);
   glutIdleFunc(display);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(normalKeys);
   glutSpecialFunc(specialKeys);
   glutMouseFunc(redirect);
   glutPassiveMotionFunc(mouseMovement); //check for mouse movement
   glutMainLoop();
   return 0;
}
