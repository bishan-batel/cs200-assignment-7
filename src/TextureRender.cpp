/**
 * Name: Kishan S Patel
 * Email: kishan.patel@digipen.edu
 * Assignment Number: 6
 * Course: CS200
 * Term: Fall 2024
 *
 * File: TextureRender.cpp
 */

#include "TextureRender.h"
#include <stdexcept>
#include <array>

namespace gl {
  using id = GLuint;

  static gl::id compile_shader( //
    const char* const source,
    const GLenum shader_type
  ) {

    const gl::id shader = glCreateShader(shader_type);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint did_shader_compile = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &did_shader_compile);

    if (did_shader_compile == GL_FALSE) {
      std::array<char, 1024> error_buff{};

      glGetShaderInfoLog(shader, sizeof(error_buff), nullptr, &error_buff[0]);

      glDeleteShader(shader);

      throw std::runtime_error{error_buff.data()};
    }
    return shader;
  }
}

static const char* const FRAGMENT_SHADER_SOURCE = R"(
#version 130
uniform sampler2D usamp;
in vec2 vtexcoord;
out vec4 frag_color;
void main(void) {
  frag_color = texture(usamp, vtexcoord);
}
)";

static const char* const VERTEX_SHADER_SOURCE = R"(
#version 130
in vec4 position;
in vec2 texcoord;
uniform mat4 transform;
out vec2 vtexcoord;
void main() {
  gl_Position = transform * position;
  vtexcoord = texcoord;
})";

cs200::TextureRender::TextureRender():
    program{0},         //
    texture_buffer{0},  //
    vertex_buffer{0},   //
    texcoord_buffer{0}, //
    face_buffer{0},     //
    vao{0},             //
    utransform{0},      //
    mesh_face_count{0} {

  const gl::id fragment_shader =
    gl::compile_shader(FRAGMENT_SHADER_SOURCE, GL_FRAGMENT_SHADER);

  const gl::id vertex_shader =
    gl::compile_shader(VERTEX_SHADER_SOURCE, GL_VERTEX_SHADER);

  program = glCreateProgram();

  glAttachShader(program, fragment_shader);
  glAttachShader(program, vertex_shader);

  glLinkProgram(program);

  GLint did_program_link = 0;

  glGetProgramiv(program, GL_LINK_STATUS, &did_program_link);

  if (did_program_link == GL_FALSE) {
    std::array<char, 1024> error_buff{};

    glGetProgramInfoLog(program, 0, nullptr, &error_buff[0]);

    glDeleteProgram(program);
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);

    throw std::runtime_error{error_buff.data()};
  }

  glDeleteShader(fragment_shader);
  glDeleteShader(vertex_shader);

  utransform = glGetUniformLocation(program, "transform");
}

cs200::TextureRender::~TextureRender() {
  glUseProgram(0);
  glDeleteProgram(program);
}

void cs200::TextureRender::clearFrame(const glm::vec4& c) {
  glClearColor(c.r, c.g, c.b, c.a);
  glClear(GL_COLOR_BUFFER_BIT);
}

void cs200::TextureRender::loadTexture(
  unsigned char* const rgbdata, const int width, const int height
) {
  glGenTextures(1, &texture_buffer);
  glBindTexture(GL_TEXTURE_2D, texture_buffer);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    width,
    height,
    0,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    rgbdata
  );

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void cs200::TextureRender::setTransform(const glm::mat4& transform_matrix) {
  glUseProgram(program);
  glUniformMatrix4fv(utransform, 1, false, &transform_matrix[0][0]);
  glUseProgram(0);
}

void cs200::TextureRender::loadMesh(const TexturedMesh& m) {
  // Updating the mesh count variables needed here
  mesh_face_count = m.faceCount();

  // Binding vertex buffer
  {
    glGenBuffers(1, &texcoord_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer);

    glBufferData(
      GL_ARRAY_BUFFER,
      (GLsizeiptr)(sizeof(glm::vec2)) * m.vertexCount(),
      m.texcoordArray(),
      GL_STATIC_DRAW
    );
  }

  // Binding vertex buffer
  {

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    // Sending data to GPU
    glBufferData(
      GL_ARRAY_BUFFER,
      static_cast<std::intptr_t>(m.vertexCount() * sizeof(glm::vec4)),
      m.vertexArray(),
      GL_STATIC_DRAW
    );
  }

  // Binding face buffer
  {
    glGenBuffers(1, &face_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);

    // Sending data to GPU
    glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      static_cast<std::intptr_t>(m.faceCount() * sizeof(Mesh::Face)),
      m.faceArray(),
      GL_STATIC_DRAW
    );
  }

  // create VAO
  {
    glGenVertexArrays(1, &vao);
    // Getting the attributes for the VAOs
    const GLint position_attribute = glGetAttribLocation(program, "position"),
                atexcoord = glGetAttribLocation(program, "texcoord");

    glBindVertexArray(vao);

    // vert buff
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(
      position_attribute, 4, GL_FLOAT, false, sizeof(glm::vec4), nullptr
    );
    glEnableVertexAttribArray(position_attribute);

    // texcoord buff
    glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer);
    glVertexAttribPointer(atexcoord, 2, GL_FLOAT, false, 0, nullptr);
    glEnableVertexAttribArray(atexcoord);

    // index buff
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }
}

void cs200::TextureRender::unloadTexture() {
  if (texture_buffer) {
    glDeleteTextures(1, &texture_buffer);
    texture_buffer = 0;
  }
}

void cs200::TextureRender::unloadMesh() {

  mesh_face_count = 0;

  if (face_buffer) {
    glDeleteBuffers(1, &face_buffer);
    face_buffer = 0;
  }

  if (vertex_buffer) {
    glDeleteBuffers(1, &vertex_buffer);
    vertex_buffer = 0;
  }

  if (vao) {
    glDeleteVertexArrays(1, &vao);
    vao = 0;
  }
}

void cs200::TextureRender::displayFaces() {
  glUseProgram(program);
  glBindVertexArray(vao);
  glBindTexture(GL_TEXTURE_2D, texture_buffer);

  glDrawElements(GL_TRIANGLES, mesh_face_count * 3, GL_UNSIGNED_INT, nullptr);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}
