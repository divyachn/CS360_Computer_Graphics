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

void init (void) {
  glClearColor(1.0, 1.0, 1.0, 1.0); // set the background color - white
  glEnable (GL_DEPTH_TEST); //enable the depth testing

  // Specify the light source and its properties
  GLfloat light_ambient[] = {0.0, 0.0, 0.0, 1.0}; // no ambient light
  GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0}; // bright white light
  GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};  // bright white light
  GLfloat light_position1[] = {10.0, 20.0, 0.0, 1.0}; // light source 1 on ceiling
  GLfloat light_position2[] = {-10.0, 20.0, 0.0, 1.0};  // light source 2 on ceiling
  GLfloat spot_direction[] = { -1.0, -1.0, 0.0 };

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position1);

  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);
  glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0.0);
  glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0);

  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0);
  glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0);

  glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT1, GL_POSITION, light_position2);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);

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

void drawPolygon(struct vector_def vertices[], struct vector_def t_coord[], int n, char* bmp_filename){
  // for a polygon specify the texture map to be mapped
  text = loadBMP_custom(bmp_filename);
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

void draw_room(){
  struct vector_def t[] = {formVertex(0.0, 0.0, 0.0), formVertex(0.0, 1.0, 0.0), formVertex(1.0, 1.0, 0.0), formVertex(1.0, 0.0, 0.0)};
  // x constant, blue color
  // glColor3f(0.0, 0.0, 1.0);
  struct vector_def face1[] = {formVertex(20.0, 20.0, 20.0), formVertex(20.0, 20.0, -20.0), formVertex(20.0, -20.0, -20.0), formVertex(20.0, -20.0, 20.0)};
  struct vector_def face2[] = {formVertex(-20.0, 20.0, 20.0), formVertex(-20.0, 20.0, -20.0), formVertex(-20.0, -20.0, -20.0), formVertex(-20.0, -20.0, 20.0)};
  drawPolygon(face1, t, 4, (char *) "carpet.bmp");
  drawPolygon(face2, t, 4, (char *) "carpet.bmp");
  // z constant, green color
  // glColor3f(0.0, 1.0, 0.0);
  struct vector_def face3[] = {formVertex(20.0, 20.0, 20.0), formVertex(20.0, -20.0, 20.0), formVertex(-20.0, -20.0, 20.0), formVertex(-20.0, 20.0, 20.0)};
  struct vector_def face4[] = {formVertex(20.0, 20.0, -20.0), formVertex(20.0, -20.0, -20.0), formVertex(-20.0, -20.0, -20.0), formVertex(-20.0, 20.0, -20.0)};
  drawPolygon(face3, t, 4, (char *) "carpet.bmp");
  drawPolygon(face4, t, 4, (char *) "carpet.bmp");
  // y constant, red color
  // glColor3f(1.0, 0.0, 0.0);
  struct vector_def face5[] = {formVertex(20.0, 20.0, 20.0), formVertex(20.0, 20.0, -20.0), formVertex(-20.0, 20.0, -20.0), formVertex(-20.0, 20.0, 20.0)};
  struct vector_def face6[] = {formVertex(20.0, -20.0, 20.0), formVertex(20.0, -20.0, -20.0), formVertex(-20.0, -20.0, -20.0), formVertex(-20.0, -20.0, 20.0)};
  drawPolygon(face5, t, 4, (char *) "carpet.bmp");
  drawPolygon(face6, t, 4, (char *) "carpet.bmp");
}

void display (void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    struct vector_def camera_coordinates = cam.getCameraCoordinates();
    struct vector_def look_at = cam.getLookAtVector();
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
