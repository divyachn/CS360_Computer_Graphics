/* So far when shader-loader was not included, following command was used to compile:
$ g++ main.cpp -lGL -lGLU -lglut -lm
It generated the a.out executable file.
*/

/* Now we have a .cpp file which contains the code to load the shader file. So use the following commands to compile:
$ g++ -ggdb -std=c++11 -c -o shader_utils.o shader_utils.cpp
$ g++ -ggdb -std=c++11 main.cpp Camera.cpp shader_utils.o -lglut -lGLEW -lGL -lGLU -lm -o room
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
#include "Camera.cpp"

using namespace std;

GLuint program;
GLuint text;
GLuint TextureID = 0;

GLint attribute_v_coord = -1;
GLint attribute_v_normal = -1;
GLint attribute_v_color = -1;
GLint uniform_m = -1, uniform_v = -1, uniform_p = -1;
GLint uniform_m_3x3_inv_transp = -1, uniform_v_inv = -1;

char* bmp_filename;

class Mesh {

private:
    GLuint vbo_vertices, vbo_normals, vbo_texcoords, ibo_elements;

public:
    vector <glm::vec4> vertices;
    vector <glm::vec3> normals;
    vector <glm::vec2> tex_coords;
    vector <GLushort> elements;
    glm::mat4 object2world;

    // default constructor
    Mesh() : vbo_vertices(0), vbo_normals(0), vbo_texcoords(0), ibo_elements(0), object2world(glm::mat4(1)) {};

    // destructor
    ~Mesh() {
        if (vbo_vertices != 0) { glDeleteBuffers(1, &vbo_vertices); }
        if (vbo_normals != 0) { glDeleteBuffers(1, &vbo_normals); }
        if (ibo_elements != 0) { glDeleteBuffers(1, &ibo_elements); }
        if (vbo_texcoords != 0) { glDeleteBuffers(1, &vbo_texcoords); }
    }

    /* Store object vertices, normals, color and/or elements in graphic card buffers */
    void upload() {

        if (this->vertices.size() > 0) {
            glGenBuffers(1, &this->vbo_vertices);
            glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);
            glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(this->vertices[0]),
                         this->vertices.data(), GL_STATIC_DRAW);
        }
        if (this->normals.size() > 0) {
            glGenBuffers(1, &this->vbo_normals);
            glBindBuffer(GL_ARRAY_BUFFER, this->vbo_normals);
            glBufferData(GL_ARRAY_BUFFER, this->normals.size() * sizeof(this->normals[0]),
                         this->normals.data(), GL_STATIC_DRAW);
        }
        if (this->elements.size() > 0) {
            glGenBuffers(1, &this->ibo_elements);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo_elements);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->elements.size() * sizeof(this->elements[0]),
                         this->elements.data(), GL_STATIC_DRAW);
        }

        for (size_t i = 0; i < this->vertices.size(); i++) {
            float s, t;
            s = ((float) (rand() % 1000 )) / 1000;
            t = ((float) (rand() % 1000 )) / 1000;
            this->tex_coords.push_back(glm::vec2(s, t));
        }

        // if (this->tex_coords.size() > 0) {
        //     text = loadBMP_custom(bmp_filename);
        //     const char* uniform_name;
        //     uniform_name = "tex";
        //     TextureID = glGetUniformLocation(program, uniform_name);
        //     glGenBuffers(1, &this->vbo_texcoords);
        //     glBindBuffer(GL_ARRAY_BUFFER, this->vbo_texcoords);
        //     glBufferData(GL_ARRAY_BUFFER, this->tex_coords.size() * sizeof(this->tex_coords[0]),
        //                  this->tex_coords.data(), GL_STATIC_DRAW);
        // }
    }

    /* Draw the object */
    // void draw() {
    //
    //     if (this->vbo_vertices != 0) {
    //         glEnableVertexAttribArray(attribute_v_coord);
    //         glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);
    //         glVertexAttribPointer(
    //             attribute_v_coord,  // attribute
    //             4,                  // number of elements per vertex, here (x,y,z,w)
    //             GL_FLOAT,           // the type of each element
    //             GL_FALSE,           // take our values as-is
    //             0,                  // no extra data between each position
    //             0                   // offset of first element
    //         );
    //     }
    //     if (this->vbo_normals != 0) {
    //         glEnableVertexAttribArray(attribute_v_normal);
    //         glBindBuffer(GL_ARRAY_BUFFER, this->vbo_normals);
    //         glVertexAttribPointer(
    //             attribute_v_normal, // attribute
    //             3,                  // number of elements per vertex, here (x,y,z)
    //             GL_FLOAT,           // the type of each element
    //             GL_FALSE,           // take our values as-is
    //             0,                  // no extra data between each position
    //             0                   // offset of first element
    //         );
    //     }
    //
    //     glActiveTexture(GL_TEXTURE0);
    //     glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    //     glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    //     glBindTexture(GL_TEXTURE_2D, text);
    //     glUniform1i(TextureID, 0);
    //
    //     /* Apply object's transformation matrix */
    //     glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(this->object2world));
    //     /* Transform normal vectors with transpose of inverse of upper left
    //        3x3 model matrix (ex-gl_NormalMatrix): */
    //     glm::mat3 m_3x3_inv_transp = glm::transpose(glm::inverse(glm::mat3(this->object2world)));
    //     glUniformMatrix3fv(uniform_m_3x3_inv_transp, 1, GL_FALSE, glm::value_ptr(m_3x3_inv_transp));
    //
    //     /* Push each element in buffer_vertices to the vertex shader */
    //     if (this->ibo_elements != 0) {
    //         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo_elements);
    //         int size;
    //         glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    //         glDrawElements(GL_TRIANGLES, size / sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
    //     } else {
    //         glDrawArrays(GL_TRIANGLES, 0, this->vertices.size());
    //     }
    //
    //     if (this->vbo_normals != 0) { glDisableVertexAttribArray(attribute_v_normal); }
    //     if (this->vbo_vertices != 0) { glDisableVertexAttribArray(attribute_v_coord); }
    // }

    /* Draw object bounding box */
    // void draw_bbox() {
    //     if (this->vertices.size() == 0) { return; }
    //     // Cube 1x1x1, centered on origin
    //     GLfloat vertices[] = {
    //         -0.5, -0.5, -0.5, 1.0,
    //         0.5, -0.5, -0.5, 1.0,
    //         0.5,  0.5, -0.5, 1.0,
    //         -0.5,  0.5, -0.5, 1.0,
    //         -0.5, -0.5,  0.5, 1.0,
    //         0.5, -0.5,  0.5, 1.0,
    //         0.5,  0.5,  0.5, 1.0,
    //         -0.5,  0.5,  0.5, 1.0,
    //     };
    //     GLuint vbo_vertices;
    //     glGenBuffers(1, &vbo_vertices);
    //     glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
    //     glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //     glBindBuffer(GL_ARRAY_BUFFER, 0);
    //
    //     GLushort elements[] = {
    //         0, 1, 2, 3,
    //         4, 5, 6, 7,
    //         0, 4, 1, 5,
    //         2, 6, 3, 7
    //     };
    //     GLuint ibo_elements;
    //     glGenBuffers(1, &ibo_elements);
    //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
    //     glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
    //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //
    //     GLfloat min_x, max_x,
    //             min_y, max_y,
    //             min_z, max_z;
    //     min_x = max_x = this->vertices[0].x;
    //     min_y = max_y = this->vertices[0].y;
    //     min_z = max_z = this->vertices[0].z;
    //
    //     for (unsigned int i = 0; i < this->vertices.size(); i++) {
    //         if (this->vertices[i].x < min_x) { min_x = this->vertices[i].x; }
    //         if (this->vertices[i].x > max_x) { max_x = this->vertices[i].x; }
    //         if (this->vertices[i].y < min_y) { min_y = this->vertices[i].y; }
    //         if (this->vertices[i].y > max_y) { max_y = this->vertices[i].y; }
    //         if (this->vertices[i].z < min_z) { min_z = this->vertices[i].z; }
    //         if (this->vertices[i].z > max_z) { max_z = this->vertices[i].z; }
    //     }
    //
    //     glm::vec3 size = glm::vec3(max_x - min_x, max_y - min_y, max_z - min_z);
    //     glm::vec3 center = glm::vec3((min_x + max_x) / 2, (min_y + max_y) / 2, (min_z + max_z) / 2);
    //     glm::mat4 transform = glm::scale(glm::mat4(1), size) * glm::translate(glm::mat4(1), center);
    //
    //     /* Apply object's transformation matrix */
    //     glm::mat4 m = this->object2world * transform;
    //     glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(m));
    //
    //     glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
    //     glEnableVertexAttribArray(attribute_v_coord);
    //     glVertexAttribPointer(
    //         attribute_v_coord,  // attribute
    //         4,                  // number of elements per vertex, here (x,y,z,w)
    //         GL_FLOAT,           // the type of each element
    //         GL_FALSE,           // take our values as-is
    //         0,                  // no extra data between each position
    //         0                   // offset of first element
    //     );
    //
    //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
    //     glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
    //     glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4 * sizeof(GLushort)));
    //     glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(GLushort)));
    //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //
    //     glDisableVertexAttribArray(attribute_v_coord);
    //     glBindBuffer(GL_ARRAY_BUFFER, 0);
    //
    //     glDeleteBuffers(1, &vbo_vertices);
    //     glDeleteBuffers(1, &ibo_elements);
    // }
};

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

void drawPolygon(struct vector_def vertices[], int n){
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
  struct vector_def face1[] = {formVertex(20.0, 20.0, 20.0), formVertex(20.0, 20.0, -20.0), formVertex(20.0, -20.0, -20.0), formVertex(20.0, -20.0, 20.0)};
  struct vector_def face2[] = {formVertex(-20.0, 20.0, 20.0), formVertex(-20.0, 20.0, -20.0), formVertex(-20.0, -20.0, -20.0), formVertex(-20.0, -20.0, 20.0)};
  drawPolygon(face1, 4);
  drawPolygon(face2, 4);
  // z constant, green color
  glColor3f(0.0, 1.0, 0.0);
  struct vector_def face3[] = {formVertex(20.0, 20.0, 20.0), formVertex(20.0, -20.0, 20.0), formVertex(-20.0, -20.0, 20.0), formVertex(-20.0, 20.0, 20.0)};
  struct vector_def face4[] = {formVertex(20.0, 20.0, -20.0), formVertex(20.0, -20.0, -20.0), formVertex(-20.0, -20.0, -20.0), formVertex(-20.0, 20.0, -20.0)};
  drawPolygon(face3, 4);
  drawPolygon(face4, 4);
  // y constant, red color
  glColor3f(1.0, 0.0, 0.0);
  struct vector_def face5[] = {formVertex(20.0, 20.0, 20.0), formVertex(20.0, 20.0, -20.0), formVertex(-20.0, 20.0, -20.0), formVertex(-20.0, 20.0, 20.0)};
  struct vector_def face6[] = {formVertex(20.0, -20.0, 20.0), formVertex(20.0, -20.0, -20.0), formVertex(-20.0, -20.0, -20.0), formVertex(-20.0, -20.0, 20.0)};
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
