/* So far when shader-loader was not included, following command was used to compile:
$ g++ version_one.cpp -lGL -lGLU -lglut -lm
It generated the a.out executable file.
*/

/* Now we have a .cpp file which contains the code to load the shader file. So use the following commands to compile:
$ g++ -ggdb -std=c++11 -c -o shader_utils.o shader_utils.cpp
$ g++ -ggdb -std=c++11 -c -o texture.o texture.cpp
$ g++ -ggdb -std=c++11 main.cpp Camera.cpp shader_utils.o texture.o -lglut -lGLEW -lGL -lGLU -lm -o room
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <GL/glew.h>
#include <GL/glut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_utils.h"
#include "texture.hpp"
#include "Camera.cpp"

using namespace std;

GLuint program;

GLint attribute_v_coord = -1;
GLint attribute_v_normal = -1;
GLint attribute_v_color = -1;
GLint attribute_v_texture = -1;
GLint uniform_m = -1, uniform_v = -1, uniform_p = -1;
GLint uniform_m_3x3_inv_transp = -1, uniform_v_inv = -1;

GLuint text;
GLuint TextureID = 0;

Camera cam;

struct vector_def formVertex(float x, float y, float z){
  struct vector_def v;
  v.x=x;
  v.y=y;
  v.z=z;
  return v;
}

int compile_link_shaders(char* vshader_filename, char* fshader_filename){
    GLint link_ok = GL_FALSE;
    GLint validate_ok = GL_FALSE;
    GLuint vs, fs;

    if ((vs = create_shader(vshader_filename, GL_VERTEX_SHADER))   == 0) { return 0; }
    if ((fs = create_shader(fshader_filename, GL_FRAGMENT_SHADER)) == 0) { return 0; }

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);

    if (!link_ok) {
        fprintf(stderr, "glLinkProgram:");
        print_log(program);
        return 0;
    }

    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &validate_ok);

    if (!validate_ok) {
        fprintf(stderr, "glValidateProgram:");
        print_log(program);
    }

    const char* attribute_name;

    attribute_name = "v_coord";
    attribute_v_coord = glGetAttribLocation(program, attribute_name);
    if (attribute_v_coord == -1) {
        fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
        return 0;
    }

    attribute_name = "v_normal";
    attribute_v_normal = glGetAttribLocation(program, attribute_name);
    if (attribute_v_normal == -1) {
        fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
        return 0;
    }

    attribute_name = "v_color";
    attribute_v_color = glGetAttribLocation(program, attribute_name);
    if (attribute_v_color == -1) {
        fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
        return 0;
    }

    attribute_name = "v_texture";
    attribute_v_texture = glGetAttribLocation(program, attribute_name);
    if (attribute_v_texture == -1) {
        fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
        return 0;
    }

    const char* uniform_name;

    uniform_name = "m";
    uniform_m = glGetUniformLocation(program, uniform_name);
    if (uniform_m == -1) {
        fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
        return 0;
    }

    uniform_name = "v";
    uniform_v = glGetUniformLocation(program, uniform_name);
    if (uniform_v == -1) {
        fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
        return 0;
    }

    uniform_name = "p";
    uniform_p = glGetUniformLocation(program, uniform_name);
    if (uniform_p == -1) {
        fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
        return 0;
    }

    uniform_name = "m_3x3_inv_transp";
    uniform_m_3x3_inv_transp = glGetUniformLocation(program, uniform_name);
    if (uniform_m_3x3_inv_transp == -1) {
        fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
        return 0;
    }

    uniform_name = "v_inv";
    uniform_v_inv = glGetUniformLocation(program, uniform_name);
    if (uniform_v_inv == -1) {
        fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
        return 0;
    }

    return 1;
}

void free_resources() { glDeleteProgram(program); }

void enableLighting(){
  // Specify the light source and its properties
  GLfloat light_ambient[] = {1.0, 1.0, 1.0, 1.0}; // white ambient light
  GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0}; // bright white light
  GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};  // bright white light
  GLfloat spot_direction[] = { -1.0, -1.0, 0.0 };

  GLfloat light_position1[] = {0.0, 35.0, 0.0, 1.0};  // above the center of the room, on ceiling
  GLfloat light_position2[] = {-20.0, 35.0, -20.0, 1.0};
  GLfloat light_position3[] = {20.0, 35.0, -20.0, 1.0};

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position1);
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);

  glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT1, GL_POSITION, light_position2);
  glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);

  glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT2, GL_POSITION, light_position3);
  glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spot_direction);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHT2);
}

void init (void) {
  glClearColor(1.0, 1.0, 1.0, 1.0); // set the background color - white
  glEnable (GL_DEPTH_TEST); //enable the depth testing
  enableLighting();
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
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

void drawPolygon(vector <struct vector_def> vertices, vector <struct vector_def> t_coord, int n){
  // for a polygon specify the texture map to be mapped
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, text);

  glPushMatrix();
  glBegin(GL_POLYGON);
    for(int i=0;i<n;i++){
      glTexCoord2f(t_coord[i].x, t_coord[i].y);
      glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
    }
  glEnd();
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
}

void const_x_polygon(vector <struct vector_def> t, float x, float d1, float d2, float y_max, float y_min, float z_max, float z_min){
  vector <struct vector_def> face;
  face.push_back(formVertex(x+d1, y_min, z_min));
  face.push_back(formVertex(x+d2, y_min, z_max));
  face.push_back(formVertex(x+d2, y_max, z_max));
  face.push_back(formVertex(x+d1, y_max, z_min));
  drawPolygon(face, t, 4);
}

void const_y_polygon(vector <struct vector_def> t, float x_max, float x_min, float y, float z_max, float z_min){
  vector <struct vector_def> face;
  face.push_back(formVertex(x_max, y, z_min));
  face.push_back(formVertex(x_min, y, z_min));
  face.push_back(formVertex(x_min, y, z_max));
  face.push_back(formVertex(x_max, y, z_max));
  drawPolygon(face, t, 4);
}

void const_z_polygon(vector <struct vector_def> t, float x_max, float x_min, float y_max, float y_min, float z){
  vector <struct vector_def> face;
  face.push_back(formVertex(x_max, y_min, z));
  face.push_back(formVertex(x_min, y_min, z));
  face.push_back(formVertex(x_min, y_max, z));
  face.push_back(formVertex(x_max, y_max, z));
  drawPolygon(face, t, 4);
}

void model_board_face(float z_plane, float wall_width, float wall_height, float gboard_width, float gboard_height, float wboard_width, float wboard_height){
  // Model the board side
  vector <struct vector_def> face, t;
  float x_max, x_min, y_max, y_min, z_delta=0.01;

  // brown wall
  x_max = wall_width/2; x_min = -x_max;
  y_max = wall_height/2; y_min = -y_max;
  t.push_back(formVertex(0.0, 0.0, 0.0));
  t.push_back(formVertex(1.0, 0.0, 0.0));
  t.push_back(formVertex(1.0, 1.0, 0.0));
  t.push_back(formVertex(0.0, 1.0, 0.0));
  text = loadBMP_custom((char *) "board_wall.bmp");
  const_z_polygon(t, x_max, x_min, y_max, y_min, z_plane);

  // green board
  x_max = gboard_width/2; x_min = -x_max;
  y_max = gboard_height/2; y_min = -y_max;
  t[0] = formVertex(0.02, 0.0, 0.0);
  t[1] = formVertex(1.0, 0.0, 0.0);
  t[2] = formVertex(1.0, 0.9, 0.0);
  t[3] = formVertex(0.02, 1.0, 0.0);
  text = loadBMP_custom((char *) "green_board.bmp");
  const_z_polygon(t, x_max, x_min, y_max, y_min, z_plane-z_delta);

  y_max = y_min+wboard_height;
  text = loadBMP_custom((char *) "white_board.bmp");
  t[0] = formVertex(0.0, 0.0, 0.0);
  t[1] = formVertex(1.0, 0.05, 0.0);
  t[2] = formVertex(0.97, 0.96, 0.0);
  t[3] = formVertex(0.1, 0.98, 0.0);
  // whiteBoard - 1 (towards positive side of X-axis)
  float imin = x_max+1.0, imax = imin+wboard_width;
  const_z_polygon(t, imax, imin, y_max, y_min, z_plane-z_delta);
  // whiteBoard - 2(towards negative side of X-axis)
  imax = x_min-1.0; imin = imax-wboard_width;
  const_z_polygon(t, imax, imin, y_max, y_min, z_plane-z_delta);
}

void model_side_faces(float const_x, float y_max, float y_min, float z_max, float straight_wall_length, float deviation, float inclined_wall_length){

  text = loadBMP_custom((char *) "wall_near_board.bmp");
  vector <struct vector_def> t;
  t.push_back(formVertex(0.0, 0.0, 0.0));
  t.push_back(formVertex(1.0, 0.0, 0.0));
  t.push_back(formVertex(1.0, 1.0, 0.0));
  t.push_back(formVertex(0.0, 1.0, 0.0));
  const_x_polygon(t, const_x, 0.0, 0.0, y_max, y_min, z_max, z_max-straight_wall_length);
  const_x_polygon(t, -const_x, 0.0, 0.0, y_max, y_min, z_max, z_max-straight_wall_length);
  const_x_polygon(t, const_x, deviation, 0.0, y_max, y_min, z_max-straight_wall_length, z_max-straight_wall_length-inclined_wall_length);
  const_x_polygon(t, -const_x, -deviation, 0.0, y_max, y_min, z_max-straight_wall_length, z_max-straight_wall_length-inclined_wall_length);
}

void model_stage(float wall_width, float floor_base, float z, float stage_height, float stage_width){
  vector <struct vector_def> t;
  t.push_back(formVertex(0.0, 0.0, 0.0));
  t.push_back(formVertex(1.0, 0.0, 0.0));
  t.push_back(formVertex(1.0, 1.0, 0.0));
  t.push_back(formVertex(0.0, 1.0, 0.0));

  // model the part along the height (i.e. along the y-axis, with z constant)
  text = loadBMP_custom((char *) "stage_height.bmp");
  const_z_polygon(t, wall_width/2, -wall_width/2, floor_base+stage_height, floor_base, z-stage_width);

  // model the top of the stage -> y is constant
  text = loadBMP_custom((char *) "stage_floor.bmp");
  const_y_polygon(t, wall_width/2, -wall_width/2, floor_base+stage_height, z, z-stage_width);
}

void model_floor(float wall_width, float floor_base, float floor_inclination, float deviation, float z,
                 float stage_width, float stage_height, float straight_wall_length, float room_length,
                 float ramp_width) {

  model_stage(wall_width, floor_base, z, stage_height, stage_width);

  // floor = flat part(constant y) + inclined part(y at one end vaires from the other end)
  vector <struct vector_def> t, face;
  t.push_back(formVertex(0.0, 0.0, 0.0));
  t.push_back(formVertex(1.0, 0.0, 0.0));
  t.push_back(formVertex(1.0, 1.0, 0.0));
  t.push_back(formVertex(0.0, 1.0, 0.0));

  text = loadBMP_custom((char *) "floor.bmp");
  const_y_polygon(t, wall_width/2, -wall_width/2, floor_base, z-stage_width, z-straight_wall_length);

  face.push_back(formVertex(wall_width/2+deviation, floor_base+floor_inclination, z-room_length));
  face.push_back(formVertex(-wall_width/2-deviation, floor_base+floor_inclination, z-room_length));
  face.push_back(formVertex(-wall_width/2, floor_base, z-straight_wall_length));
  face.push_back(formVertex(wall_width/2, floor_base, z-straight_wall_length));
  drawPolygon(face, t, 4);

  face[0] = formVertex(ramp_width/2, floor_base+floor_inclination+0.01, z-room_length);
  face[1] = formVertex(-ramp_width/2, floor_base+floor_inclination+0.01, z-room_length);
  face[2] = formVertex(-ramp_width/2, floor_base+0.01, z-straight_wall_length);
  face[3] = formVertex(ramp_width/2, floor_base+0.01, z-straight_wall_length);
  text = loadBMP_custom((char *) "carpet.bmp");
  drawPolygon(face, t, 4);
}

void model_door_face(float x_max, float x_min, float y_max, float y_min, float z, float door_width){
  vector <struct vector_def> t;
  text = loadBMP_custom((char *) "back_wall.bmp");
  t.push_back(formVertex(0.0, 0.0, 0.0));
  t.push_back(formVertex(1.0, 0.0, 0.0));
  t.push_back(formVertex(1.0, 1.0, 0.0));
  t.push_back(formVertex(0.0, 1.0, 0.0));

  // left part of back wall
  const_z_polygon(t, x_max, door_width/2, y_max, y_min, z);

  // right part of back wall, flip the image
  t[0] = formVertex(1.0, 0.0, 0.0);
  t[1] = formVertex(0.0, 0.0, 0.0);
  t[2] = formVertex(0.0, 1.0, 0.0);
  t[3] = formVertex(1.0, 1.0, 0.0);
  const_z_polygon(t, -door_width/2, x_min, y_max, y_min, z);

  text = loadBMP_custom((char *) "entrance.bmp");
  t[0] = formVertex(0.1, 0.0, 0.0);
  t[1] = formVertex(1.0, 0.0, 0.0);
  t[2] = formVertex(1.0, 1.0, 0.0);
  t[3] = formVertex(0.1, 1.0, 0.0);
  const_z_polygon(t, door_width/2, -door_width/2, y_max, y_min, z);
}

void model_chairs(float z_max, float z_min, float y_max, float y_min, float wall_width, float ramp_width, float chair_width, float chair_height, float chair_length){
  vector <struct vector_def> t;
  t.push_back(formVertex(0.0, 0.0, 0.0));
  t.push_back(formVertex(1.0, 0.0, 0.0));
  t.push_back(formVertex(1.0, 1.0, 0.0));
  t.push_back(formVertex(0.0, 1.0, 0.0));

  // CHAIR: length(Z-axis) width(X-axis) height(Y-axis)

  // inclination-equation : how y vaires as we move from z=z_max to z=z_min
  float inclination = y_max-y_min;
  float slope = inclination/(z_min - z_max);  // negative slope
  float intercept = y_min - (slope*z_max);

  float z_start = z_max-2.0, z_end = z_min+2.0, z_incr = chair_length+3.0, margin=2.0;  // margin is the space between first chair and ramp
  float x, y, z, gap=7.0; // gap is the space between two chairs
  int num_chairs = 5;

  text = loadBMP_custom((char *) "chair_seat.bmp");
  // flat surface - where we sit => y is constant
  for(z=z_start; z>=z_end; z-=z_incr){
    // for each row i.e. given some (y,z) vary the x and model the flat surface of the chair for both the halves
    y = slope*z + intercept;
    x = ramp_width/2 + margin + chair_width/2;
    for(int i=0; i<num_chairs; i++){  // model chairs for each row
      const_y_polygon(t, x+chair_width/2, x-chair_width/2, y, z+chair_length, z); // Model the right-half
      const_y_polygon(t, -x+chair_width/2, -x-chair_width/2, y, z+chair_length, z); // Model the left-half
      x = x+gap;
    }
  }

  text = loadBMP_custom((char *) "chair_support.bmp");
  // support surface - where we sit => z is constant
  for(z=z_start; z>=z_end; z-=z_incr){
    // for each row i.e. given some (y,z) vary the x and model the flat surface of the chair for both the halves
    y = slope*z + intercept;
    x = ramp_width/2 + margin + chair_width/2;
    for(int i=0; i<num_chairs; i++){  // model chairs for each row
      const_z_polygon(t, x+chair_width/2, x-chair_width/2, y+chair_height, y, z); // Model the right-half
      const_z_polygon(t, -x+chair_width/2, -x-chair_width/2, y+chair_height, y, z); // Model the left-half
      x = x+gap;
    }
  }

}

void model_ceiling(float wall_width, float deviation, float y, float z_max, float z_min){
  // plain white roof
  vector <struct vector_def> face;
  face.push_back(formVertex(wall_width/2, y, z_max));
  face.push_back(formVertex(-wall_width/2, y, z_max));
  face.push_back(formVertex(-wall_width/2-deviation, y, z_min));
  face.push_back(formVertex(wall_width/2+deviation, y, z_min));
  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_POLYGON);
    for(int i=0;i<4;i++){
        glVertex3f(face[i].x, face[i].y, face[i].z);
    }
  glEnd();
}

void model_room(){

  float z_far_plane = 40.0, wall_width=100.0, wall_height=80.0;
  float green_board_width = 50.0, green_board_height = 46.0;
  float white_board_width = 12.0, white_board_height = 30.0;
  model_board_face(z_far_plane, wall_width, wall_height, green_board_width, green_board_height, white_board_width, white_board_height);

  float const_x = wall_width/2, y_max = wall_height/2, y_min = -y_max;
  float inclined_wall_length = 60.0, straight_wall_length = 20.0;
  float deviation = 5.0;  // how much x of -ve z extreme will differ from the +ve z-extreme
  model_side_faces(const_x, y_max, y_min, z_far_plane, straight_wall_length, deviation, inclined_wall_length); // side_wall (positive X-axis)

  float stage_height = 8.0, stage_width = straight_wall_length*0.80;
  float ramp_width = 10.0;
  float floor_inclination = 10.0;
  model_floor(wall_width, -wall_height/2, floor_inclination, deviation, z_far_plane,
    stage_width, stage_height, straight_wall_length, straight_wall_length+inclined_wall_length, ramp_width);

  float door_width = 2*ramp_width;
  model_door_face(wall_width/2+deviation, -wall_width/2-deviation, wall_height/2, (-wall_height)/2+floor_inclination, z_far_plane-straight_wall_length-inclined_wall_length, door_width);

  model_ceiling(wall_width, deviation, wall_height/2, z_far_plane, z_far_plane-straight_wall_length-inclined_wall_length);

  float z_max=z_far_plane-straight_wall_length;
  float z_min=z_far_plane-straight_wall_length-inclined_wall_length;
  float chair_width=4.0, chair_height=5.0, chair_length=3.0; // length(Z-axis) width(X-axis) height(Y-axis)
  model_chairs(z_max, z_min, -wall_height/2+floor_inclination, -wall_height/2, wall_width, ramp_width, chair_width, chair_height, chair_length);
}

void display (void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    struct vector_def camera_coordinates = cam.getCameraCoordinates();
    struct vector_def look_at = cam.getLookAtVector();
    gluLookAt(camera_coordinates.x, camera_coordinates.y, camera_coordinates.z,
              look_at.x, look_at.y, look_at.z,
              0.0, 1.0, 0.0);
    model_room();
    glFlush();
    glutSwapBuffers();
}

void reshape (int w, int h) {
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-40.0, 40.0, -35.0, 35.0, 20.0, 60.0);
   glMatrixMode(GL_MODELVIEW);
}

int main (int argc, char **argv) {
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize (1000,600);
    glutInitWindowPosition (120, 50);
    glutCreateWindow ("Shaders included");

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
        return 1;
    }

    // specify the vertex shader and fragment shader, files are hard-coded
    char* v_shader_filename = (char*) "./vertex_shader.v.glsl";
    char* f_shader_filename = (char*) "./phong_shading.f.glsl";

    int response = compile_link_shaders(v_shader_filename, f_shader_filename);
    if(response==0) return 0;

    init ();
    glutDisplayFunc (display);
    glutReshapeFunc (reshape);
    glutKeyboardFunc(normalKeys);
    glutSpecialFunc(specialKeys);
    glutMainLoop ();
    free_resources();
    return 0;
}
