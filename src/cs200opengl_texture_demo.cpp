// cs200opengl_texture_demo.cpp
// -- Draw a textured rotating square using OpenGL with GLM, GLEW, and SDL
// cs200 10/20
//
// To compile using Visual Studio command prompt:
//   cl /EHsc cs200opengl_texture_demo.cpp opengl32.lib glew32.lib
//      SDL2.lib SDL2main.lib /link /subsystem:console
// To compile/link on Linux:
//   g++ cs200opengl_texture_demo.cpp -lSDL2 -lGL -lGLEW
//
// Note:
//   The file 'texture.bmp' is assumed to exist

#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
using namespace std;


class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(double dt);
    void keypress(SDL_Keycode kc);
    void resize(int W, int H);
    void mouseclick(int x, int y);
  private:
    GLuint program,
           vao1,
           vao2,
           texture_buffer,
           vertex_buffer,
           texcoord_buffer1,
           texcoord_buffer2,
           face_buffer;
    GLint utransform;
    double rotation_time;
    int mode;
};


Client::Client(void)
    : rotation_time(0),
      mode(0) {
  GLint value;

  // compile and link shaders
  const char *fragment_shader_text =
    "#version 130\n\
     uniform sampler2D usamp;\
     in vec2 vtexcoord;\
     out vec4 frag_color;\
     void main(void) {\
       frag_color = texture(usamp,vtexcoord);\
     }";
  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fshader,1,&fragment_shader_text,0);
  glCompileShader(fshader);
  glGetShaderiv(fshader,GL_COMPILE_STATUS,&value);
  if (!value) {
    char buffer[1024];
    glGetShaderInfoLog(fshader,1024,0,buffer);
    cerr << buffer << endl;
  }

  const char *vertex_shader_text =
    "#version 130\n\
     in vec4 position;\
     in vec2 texcoord;\
     uniform mat4 transform;\
     out vec2 vtexcoord;\
     void main() {\
       gl_Position = transform * position;\
       vtexcoord = texcoord;\
     }";
  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vshader,1,&vertex_shader_text,0);
  glCompileShader(vshader);
  glGetShaderiv(vshader,GL_COMPILE_STATUS,&value);
  if (!value) {
    char buffer[1024];
    glGetShaderInfoLog(vshader,1024,0,buffer);
    cerr << buffer << endl;
  }

  program = glCreateProgram();
  glAttachShader(program,fshader);
  glAttachShader(program,vshader);
  glLinkProgram(program);
  glGetProgramiv(program,GL_LINK_STATUS,&value);
  if (!value) {
    cerr << "shader program failed to link" << endl;
    char buffer[1024];
    glGetProgramInfoLog(program,1024,0,buffer);
    cerr << buffer << endl;
  }
  glDeleteShader(fshader);
  glDeleteShader(vshader);

  // load bitmap file
  //   Remark:
  //     SDL (incorrectly) assumes RGB color order
  //     and top-down scanning!
  SDL_Surface *bmp_surface = SDL_LoadBMP("texture.bmp");
  if (bmp_surface == 0)
    cout << "failed to load 'texture.bmp'" << endl;
  unsigned char *bmp_data = reinterpret_cast<unsigned char*>(bmp_surface->pixels);
  int bmp_width = bmp_surface->w,
      bmp_height = bmp_surface->h;

  // texture buffer
  glGenTextures(1,&texture_buffer);
  glBindTexture(GL_TEXTURE_2D,texture_buffer);
  glPixelStorei(GL_UNPACK_ALIGNMENT,4);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,bmp_width,bmp_height,
               0,GL_RGB,GL_UNSIGNED_BYTE,bmp_data);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

  // delete bitmap data (on CPU)
  SDL_FreeSurface(bmp_surface);

  // vertex buffer for standard square
  glGenBuffers(1,&vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  float vertices[16] = { 1,1,0,1, -1,1,0,1, -1,-1,0,1, 1,-1,0,1 };
  glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

  // texture coordinate buffer #1 (non-tiling)
  glGenBuffers(1,&texcoord_buffer1);
  glBindBuffer(GL_ARRAY_BUFFER,texcoord_buffer1);
  float texcoords1[8] = { 1,1, 0,1, 0,0, 1,0 };
  glBufferData(GL_ARRAY_BUFFER,sizeof(texcoords1),texcoords1,GL_STATIC_DRAW);

  // texture coordinate buffer #2 (tiled)
  glGenBuffers(1,&texcoord_buffer2);
  glBindBuffer(GL_ARRAY_BUFFER,texcoord_buffer2);
  float texcoords2[8] = { 1.5f,1.5f, -1.5f,1.5f,
                         -1.5f,-1.5f, 1.5f,-1.5f };
  glBufferData(GL_ARRAY_BUFFER,sizeof(texcoords2),texcoords2,GL_STATIC_DRAW);

  // face list buffer
  glGenBuffers(1,&face_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,face_buffer);
  unsigned faces[6] = { 0,1,2, 0,2,3 };
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(faces),faces,GL_STATIC_DRAW);

  // VAO for first texturing
  GLint aposition = glGetAttribLocation(program,"position"),
        atexcoord = glGetAttribLocation(program,"texcoord");
  glGenVertexArrays(1,&vao1);
  glBindVertexArray(vao1);
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  glVertexAttribPointer(aposition,4,GL_FLOAT,false,0,0);
  glEnableVertexAttribArray(aposition);
  glBindBuffer(GL_ARRAY_BUFFER,texcoord_buffer1);
  glVertexAttribPointer(atexcoord,2,GL_FLOAT,false,0,0);
  glEnableVertexAttribArray(atexcoord);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,face_buffer);
  glBindVertexArray(0);

  // VAO for second texturing
  glGenVertexArrays(1,&vao2);
  glBindVertexArray(vao2);
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  glVertexAttribPointer(aposition,4,GL_FLOAT,false,0,0);
  glEnableVertexAttribArray(aposition);
  glBindBuffer(GL_ARRAY_BUFFER,texcoord_buffer2);
  glVertexAttribPointer(atexcoord,2,GL_FLOAT,false,0,0);
  glEnableVertexAttribArray(atexcoord);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,face_buffer);
  glBindVertexArray(0);

  // shader parameter location
  utransform = glGetUniformLocation(program,"transform");
}


Client::~Client(void) {
  // deallocate GPU resources
  glDeleteBuffers(1,&face_buffer);
  glDeleteBuffers(1,&texcoord_buffer1);
  glDeleteBuffers(1,&texcoord_buffer2);
  glDeleteBuffers(1,&vertex_buffer);

  glDeleteTextures(1,&texture_buffer);

  glDeleteVertexArrays(1,&vao1);
  glDeleteVertexArrays(1,&vao2);

  glUseProgram(0);
  glDeleteProgram(program);
}


void Client::draw(double dt) {
  const float PI = 4.0f * atan(1.0f),
              ROTATION_RATE = 2*PI/5.0f,
              TRANSLATION_RATE_X = 2*PI/7.3f,
              TRANSLATION_RATE_Y = 2*PI/5.9f;

  glClearColor(0.3f,0.2f,0.3f,1);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(program);

  // set vertex transformation matrix
  float c = cos(ROTATION_RATE*rotation_time),
        s = sin(ROTATION_RATE*rotation_time);
  glm::mat4 R(0);
  R[0][0] = 0.5f*c;  R[1][0] = -0.5f*s;
  R[0][1] = 0.5f*s,  R[1][1] = 0.5f*c;
  R[3][3] = 1;
  glUniformMatrix4fv(utransform,1,false,&R[0][0]);

  // select texture buffer to use
  glBindTexture(GL_TEXTURE_2D,texture_buffer);


  // select desired VAO
  if (mode/2 == 0)
    glBindVertexArray(vao1);
  else
    glBindVertexArray(vao2);

  // draw faces
  glDrawElements(GL_TRIANGLES,2*3,GL_UNSIGNED_INT,0);
  glBindVertexArray(0);

  if (mode%2 == 1)
    rotation_time += dt;
}


void Client::keypress(SDL_Keycode kc) {
  if (kc == SDLK_SPACE)
    mode = (mode + 1) % 4;
}


void Client::resize(int W, int H) {
  int D = min(W,H);
  glViewport(0,0,D,D);
}


void Client::mouseclick(int x, int y) {
  mode = (mode + 1) % 4;
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("OpenGL Demo",SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,500,500,
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
  Client *client = new Client();
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

  // clean up
  delete client;
  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

