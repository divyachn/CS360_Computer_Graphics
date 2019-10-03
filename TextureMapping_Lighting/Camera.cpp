#include <math.h>

#define XMAX 15.0
#define XMIN -15.0
#define YMAX 15.0
#define YMIN -15.0
#define ZMAX 15.0
#define ZMIN -15.0
#define PI 3.1415926535

struct vector_def{ float x,y,z; };

class Camera{

private:
  struct vector_def pos;
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
  double degreeToRadian(float angle){
    // takes angle in degree, returns angle in radians
    double radian;
    radian = PI*angle/180;
    return radian;
  }

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

  struct vector_def getCameraCoordinates(){
    return pos;
  }

  struct vector_def getLookAtVector(){
    struct vector_def look_at;
    look_at.x = 100*sin(degreeToRadian(angle_y)) * sin(degreeToRadian(angle_z));
    look_at.y = 100*cos(degreeToRadian(angle_y));
    look_at.z = 100*sin(degreeToRadian(angle_y)) * cos(degreeToRadian(angle_z));
    return look_at;
  }

};
