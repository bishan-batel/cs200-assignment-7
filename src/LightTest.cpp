// LightTest.cpp
// -- Test of spot light shader
// cs200 10/20
//

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <vector>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
#include "Camera.h"
#include "Texture.h"
#include "DPTexturedMesh.h"
using namespace std;


const glm::vec4 O = cs200::point(0,0),
                EX = cs200::vector(1,0),
                EY = cs200::vector(0,1);

float frand(float a=0, float b=1) {
  return a + (b-a)*float(rand())/float(RAND_MAX);
}


class Client {
  public:
    Client(SDL_Window *w);
    ~Client(void);
    void draw(double dt);
    void mouseclick(int x, int y);
    void keypress(SDL_Keycode kc);
    void resize(int W, int H);
  private:
    SDL_Window *window;
    vector<glm::mat4> model;
    float model_rate;
    double time;
    int mode;
    float values[3];
    enum { MOD_RADIUS=0, MOD_FACTOR=1, MOD_AMBIENT=2 };
    glm::mat4 window_to_world;
    cs200::DPTexturedMesh mesh;
    enum { VERTS=0, TEXTS=1, FACES=2 };
    GLuint program,
           texture_buffer,
           vbo[3],
           vao;
    GLint uobject_to_world,
          uworld_to_ndc,
          ulight_position,
          ulight_radius,
          ulight_factor,
          uambient_factor;
};


Client::Client(SDL_Window *w)
    : window(w) {
  program = glCreateProgram();

  // read and compile shaders
  const char *files[2] = { "light.vert", "light.frag" };
  GLenum types[2] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
  GLuint shaders[2];
  for (int i=0; i < 2; ++i) {
    ifstream in(files[i]);
    cout << "loading file " << files[i] << endl;
    string code;
    while (in) {
      string line;
      getline(in,line);
      code += line + "\n";
    }
    shaders[i] = glCreateShader(types[i]);
    const char *source = code.c_str();
    glShaderSource(shaders[i],1,&source,0);
    glCompileShader(shaders[i]);
    GLint status;
    glGetShaderiv(shaders[i],GL_COMPILE_STATUS,&status);
    if (status != GL_TRUE) {
      string msg = "*** shader '" + string(files[i]) + "' failed to compile: ***\n";
      char buffer[1024];
      glGetShaderInfoLog(shaders[i],1024,0,buffer);
      cout << buffer << endl;
    }
    glAttachShader(program,shaders[i]);
  }

  // link shaders
    glLinkProgram(program);
  GLint status;
  glGetProgramiv(program,GL_LINK_STATUS,&status);
  if (status != GL_TRUE)
    cout << "*** shader program failed to link ***" << endl;
  glDeleteShader(shaders[0]);
  glDeleteShader(shaders[1]);

  // load texture
  cs200::Bitmap *texture = nullptr;
  try {
    texture = new cs200::Bitmap("texture.bmp");
    cs200::reverseRGB(*texture);
  }
  catch (exception &e) {
    cout << "file not found: 'texture.bmp'" << endl;
    texture = new cs200::Bitmap(512,512);
  }
  glGenTextures(1,&texture_buffer);
  glBindTexture(GL_TEXTURE_2D,texture_buffer);
  glPixelStorei(GL_UNPACK_ALIGNMENT,4);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,texture->width(),texture->height(),0,
               GL_RGB,GL_UNSIGNED_BYTE,texture->data());
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  delete texture;

  // load mesh
  GLint aposition = glGetAttribLocation(program,"position"),
        atexcoord = glGetAttribLocation(program,"texcoord");
  glGenBuffers(3,vbo);

  glBindBuffer(GL_ARRAY_BUFFER,vbo[VERTS]);
  glBufferData(GL_ARRAY_BUFFER,sizeof(glm::vec4)*mesh.vertexCount(),
               mesh.vertexArray(),GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER,vbo[TEXTS]);
  glBufferData(GL_ARRAY_BUFFER,sizeof(glm::vec2)*mesh.vertexCount(),
               mesh.texcoordArray(),GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo[FACES]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(cs200::Mesh::Face)*mesh.faceCount(),
               mesh.faceArray(),GL_STATIC_DRAW);

  glGenVertexArrays(1,&vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER,vbo[VERTS]);
  glVertexAttribPointer(aposition,4,GL_FLOAT,false,0,0);
  glEnableVertexAttribArray(aposition);
  glBindBuffer(GL_ARRAY_BUFFER,vbo[TEXTS]);
  glVertexAttribPointer(atexcoord,2,GL_FLOAT,false,0,0);
  glEnableVertexAttribArray(atexcoord);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo[FACES]);
  glBindVertexArray(0);

  // shader program locations
  uobject_to_world = glGetUniformLocation(program,"object_to_world");
  uworld_to_ndc = glGetUniformLocation(program,"world_to_ndc");
  ulight_position = glGetUniformLocation(program,"light_position");
  ulight_radius = glGetUniformLocation(program,"light_radius");
  ulight_factor = glGetUniformLocation(program,"light_factor");
  uambient_factor = glGetUniformLocation(program,"ambient_factor");

  // set shader uniform values
  glUseProgram(program);
  glUniform1f(ulight_radius,3.0f);
  glUniform1f(ulight_factor,1.0f);
  glUniform1f(uambient_factor,0.2f);
  resize(500,500);
  mouseclick(250,250);

  // some modeling transforms
  for (int i=0; i < 80; ++i) {
    glm::vec4 C = cs200::point(frand(-20,20),frand(-20,20));
    float r = frand(0.1f,8.0f);
    glm::mat4 M = cs200::translate(C-O)
                  * cs200::scale(r/mesh.dimensions().x)
                  * cs200::translate(O-mesh.center());
    model.push_back(M);
  }
  model_rate = 360.0f/frand(2,5);

  // parameter adjustment stuff
  mode = 0;
  values[MOD_RADIUS] = 3.0f;
  values[MOD_FACTOR] = 1.0f;
  values[MOD_AMBIENT] = 0.2f;
}


Client::~Client(void) {
  glUseProgram(0);
  glDeleteProgram(program);

  glDeleteTextures(1,&texture_buffer);

  glDeleteBuffers(3,vbo);
  glDeleteVertexArrays(1,&vao);
}


void Client::draw(double dt) {
  glClearColor(0.2f,0.1f,0.1f,1);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(program);
  glBindTexture(GL_TEXTURE_2D,texture_buffer);
  glBindVertexArray(vao);
  for (unsigned i=0; i < model.size(); ++i) {
    glUniformMatrix4fv(uobject_to_world,1,false,&model[i][0][0]);
    glDrawElements(GL_TRIANGLES,3*mesh.faceCount(),GL_UNSIGNED_INT,0);
  }

  glm::mat4 M = cs200::rotate(model_rate*time)
                * cs200::scale(10/mesh.dimensions().x)
                * cs200::translate(O-mesh.center());
  glUniformMatrix4fv(uobject_to_world,1,false,&M[0][0]);
  glDrawElements(GL_TRIANGLES,3*mesh.faceCount(),GL_UNSIGNED_INT,0);
  glBindVertexArray(0);

  time += dt;
}


void Client::mouseclick(int x, int y) {
  glm::vec4 light_position = window_to_world * cs200::point(x,y);
  glUseProgram(program);
  glUniform4fv(ulight_position,1,&light_position[0]);
}


void Client::resize(int W, int H) {
  glViewport(0,0,W,H);
  float aspect = float(W)/float(H);
  cs200::Camera camera(O,EY,20.0f*aspect,20.0f);
  glm::mat4 world_to_ndc = cs200::cameraToNDC(camera)
                           * cs200::worldToCamera(camera);
  glUseProgram(program);
  glUniformMatrix4fv(uworld_to_ndc,1,false,&world_to_ndc[0][0]);
  window_to_world = cs200::scale(20.0f/H,-20.0f/H)
                    * cs200::translate(-0.5f*W*EX-0.5f*H*EY);
}


void Client::keypress(SDL_Keycode kc) {
  const float SCALE = 0.95f;
  const char *LABEL[3] = { "radius", "light factor", "ambient factor" };
  stringstream ss;
  glUseProgram(program);

  switch (kc) {

    case SDLK_SPACE:
      mode = (mode+1)%3;
      break;

    case SDLK_UP:
      switch (mode) {
        case MOD_RADIUS:
          values[MOD_RADIUS] /= SCALE;
          glUniform1f(ulight_radius,values[MOD_RADIUS]);
          break;
        case MOD_FACTOR:
          values[MOD_FACTOR] /= SCALE;
          glUniform1f(ulight_factor,values[MOD_FACTOR]);
          break;
        case MOD_AMBIENT:
          values[MOD_AMBIENT] /= SCALE;
          glUniform1f(uambient_factor,values[MOD_AMBIENT]);
      };
      break;

    case SDLK_DOWN:
      switch (mode) {
        case MOD_RADIUS:
          values[MOD_RADIUS] *= SCALE;
          glUniform1f(ulight_radius,values[MOD_RADIUS]);
          break;
        case MOD_FACTOR:
          values[MOD_FACTOR] *= SCALE;
          glUniform1f(ulight_factor,values[MOD_FACTOR]);
          break;
        case MOD_AMBIENT:
          values[MOD_AMBIENT] *= SCALE;
          glUniform1f(uambient_factor,values[MOD_AMBIENT]);
      }

  }

  ss << "Spot Light Shader Test [" << LABEL[mode]
     << ": " << fixed << setprecision(3) << values[mode] << "]";
  SDL_SetWindowTitle(window,ss.str().c_str());
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  const char *title = "Spot Light Shader Test [radius: 3.000]";
  int width = 500,
      height = 500;
  SDL_Window *window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,width,height,
                                        SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // GLEW: get function bindings (if possible)
  GLenum value = glewInit();
  if (value != GLEW_OK) {
    cout << glewGetErrorString(value) << endl;
    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return -1;
  }

  // animation loop
  bool done = false;
  try {
    Client *client = new Client(window);
    Uint32 ticks_last = SDL_GetTicks();
    while (!done) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_QUIT:
            done = true;
            break;
          case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
              done = true;
            else
              client->keypress(event.key.keysym.sym);
            break;
          case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
              client->resize(event.window.data1,event.window.data2);
            break;
          case SDL_MOUSEBUTTONDOWN:
            client->mouseclick(event.button.x,event.button.y);
            break;
        }
      }
      Uint32 ticks = SDL_GetTicks();
      double dt = 0.001*(ticks - ticks_last);
      ticks_last = ticks;
      client->draw(dt);
      SDL_GL_SwapWindow(window);
    }

    delete client;
  }
  catch (exception &e) {
    cout << "failed" << endl;
    cout << e.what() << endl;
  }

  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}



