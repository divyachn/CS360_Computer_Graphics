/*
$ g++ -ggdb -std=c++11 -c -o shader_utils.o shader_utils.cpp
$ g++ -ggdb -std=c++11 -c -o texture.o texture.cpp
$ g++ -ggdb -std=c++11 main.cpp shader_utils.o texture.o -lglut -lGLEW -lGL -lGLU -lm -o game
*/

#include <GL/glew.h>
#include <GL/glut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_utils.h"
#include "texture.hpp"

#include <iostream>
#include <vector>

using namespace std;

using glm::vec4;
using glm::vec3;
using glm::vec2;

#define GAME_BEGIN 1
#define GAME_OVER 2
#define GAME_WON 3
#define GAME_IN_PROGRESS 4

int gameStatus=1; // by default game starts fresh

GLuint program;

GLint attribute_v_coord = -1;
GLint attribute_v_normal = -1;
GLint attribute_v_color = -1;
GLint attribute_v_texture = -1;
GLint uniform_m = -1, uniform_v = -1, uniform_p = -1;
GLint uniform_m_3x3_inv_transp = -1, uniform_v_inv = -1;

GLuint text;
GLuint TextureID = 0;

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

void writeText(vec3 color, vec3 position, string s){
  glColor3f(color.r,color.g,color.b);
  glRasterPos3f(position.x,position.y,position.z);
  for (int i=0; i<s.size(); i++){
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
	}
}

void texturePolygon(vector <vec3> vertices, vector <vec2> t_coord, int n){
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

vector <vec3> const_z(float x_max, float x_min, float y_max, float y_min, float z){
  vector <vec3> face = {vec3(x_min,y_min,z), vec3(x_max,y_min,z), vec3(x_max,y_max,z), vec3(x_min,y_max,z)};
  return face;
}

void gameBeginScreen(){
  vector <vec2> t = {vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0)};
  float z = -20.0;
  vector <vec3> face = const_z(100.0, -100.0, 80.0, -80.0, z);
  text = loadBMP_custom((char *) "./images/maze1.bmp");
  texturePolygon(face, t, 4);
  writeText(vec3(1.0, 1.0, 1.0), vec3(-40.0, 60.0, z+0.01), "CS360 - PROJECT - A 3D maze game");
  writeText(vec3(1.0, 1.0, 1.0), vec3(-25.0, 40.0, z+0.01), "Find the DIAMOND");
  writeText(vec3(1.0, 1.0, 1.0), vec3(-20.0, 20.0, z+0.01), "Click to play!");
  writeText(vec3(1.0, 1.0, 1.0), vec3(40.0, -60.0, z+0.01), "Divya Chauhan 160246");
  writeText(vec3(1.0, 1.0, 1.0), vec3(40.0, -70.0, z+0.01), "Rahul BS 160xxx");
}

void gameOverScreen(){
  vector <vec2> t = {vec2(0.1, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.1, 1.0)};
  float z = -20.0;
  vector <vec3> face = const_z(100.0, -100.0, 80.0, -80.0, z);
  text = loadBMP_custom((char *) "./images/gameOver.bmp");
  texturePolygon(face, t, 4);
  writeText(vec3(0.0, 0.0, 0.0), vec3(-70.0, -70.0, z+0.01), "Press q to quit and r to replay.");
}

void gameWonScreen(){
  vector <vec2> t = {vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0)};
  float z = -20.0;
  vector <vec3> face = const_z(70.0, 20.0, 50.0, -50.0, z);
  text = loadBMP_custom((char *) "./images/gameWon.bmp");
  texturePolygon(face, t, 4);
  face = const_z(-10.0, -90.0, 50.0, -50.0, z);
  text = loadBMP_custom((char *) "./images/diamond.bmp");
  texturePolygon(face, t, 4);
  writeText(vec3(0.0, 0.0, 0.0), vec3(-70.0, -70.0, z+0.01), "Press q to quit and r to replay.");
}

void gameProgressScreen(){
  cout<<"Game is in progress\n";
}

void display(void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    cout<<"gameStatus: "<<gameStatus<<"\n";
    switch(gameStatus){
      case GAME_BEGIN:
        gameBeginScreen();
        break;
      case GAME_OVER:
        gameOverScreen();
        break;
      case GAME_WON:
        gameWonScreen();
        break;
      case GAME_IN_PROGRESS:
        gameProgressScreen();
        break;
      default:
        break;
    }
    glFlush ();
    glutSwapBuffers();
}

void init (void) {
  glClearColor(1.0, 1.0, 1.0, 1.0); // set the background color - white
  glEnable (GL_DEPTH_TEST); //enable the depth testing
  // enableLighting();
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void reshape (int w, int h) {
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-50.0, 50.0, -40.0, 40.0, 10, 70);
   glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize (1200,600);
  glutInitWindowPosition (100,50);
  glutCreateWindow ("Find the Diamond");

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
  glutMainLoop ();
  free_resources();
  return 0;
}
