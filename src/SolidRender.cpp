/**
 * File: SolidRender.cpp
 * Name: Kishan Patel
 * Assignment: 3
 * Course: CS200
 * Term: FALL24
 */

#include "SolidRender.h"
#include <cassert>
#include <stdexcept>
#include <string>

static const GLchar *const fragment_shader_source = "#version 130\n\
     uniform vec4 color;\
     out vec4 frag_color;\
     void main(void) {\
       frag_color = color;\
     }";

static const GLchar *const vertex_shader_source = "#version 130\n\
     in vec4 position;\
     uniform mat4 transform;\
     void main() {\
       gl_Position = transform * position;\
     }";

namespace cs200 {
  static GLuint compile_shader(const GLchar *const shader_source, GLenum shader_type) {
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_source, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
      GLint log_length = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

      std::string error_log;
      error_log.reserve(log_length);
      glGetShaderInfoLog(shader, log_length, nullptr, &error_log[0]);

      glDeleteShader(shader);

      throw std::runtime_error(error_log.data());
    }
    return shader;
  }
} // namespace cs200

cs200::SolidRender::SolidRender() :
    ucolor(0), utransform(0), program(0), vao_edges(0), vao_faces(0), vertex_buffer(0), edge_buffer(0), face_buffer(0),
    mesh_edge_count(0), mesh_face_count(0) {

  GLuint fragment_shader_id = cs200::compile_shader(fragment_shader_source, GL_FRAGMENT_SHADER);
  GLuint vertex_shader_id = cs200::compile_shader(vertex_shader_source, GL_VERTEX_SHADER);

  program = glCreateProgram();

  glAttachShader(program, fragment_shader_id);
  glAttachShader(program, vertex_shader_id);

  glLinkProgram(program);
  GLint link_ok = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
  if (link_ok == GL_FALSE) {
    GLint log_length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

    std::string error_log;
    error_log.reserve(log_length);
    glGetProgramInfoLog(program, log_length, &log_length, &error_log[0]);

    glDeleteProgram(program);
    glDeleteShader(fragment_shader_id);
    glDeleteShader(vertex_shader_id);

    throw std::runtime_error(error_log.data());
  }

  glDeleteShader(fragment_shader_id);
  glDeleteShader(vertex_shader_id);

  utransform = glGetUniformLocation(program, "transform");
  ucolor = glGetUniformLocation(program, "color");
}

cs200::SolidRender::~SolidRender() { glDeleteProgram(program); }

void cs200::SolidRender::clearFrame(const glm::vec4 &c) {
  glClearColor(c.x, c.y, c.z, c.w);
  glClear(GL_COLOR_BUFFER_BIT);
}

void cs200::SolidRender::setTransform(const glm::mat4 &M) {
  glUseProgram(program);
  glUniformMatrix4fv(utransform, 1, false, &M[0][0]);
}

void cs200::SolidRender::loadMesh(const cs200::Mesh &m) {
  // Updating the mesh count variables needed here
  mesh_edge_count = m.edgeCount();
  mesh_face_count = m.faceCount();

  // Binding vertex buffer
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  // Sending data to GPU
  glBufferData(
      GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m.vertexCount() * sizeof(glm::vec4)), m.vertexArray(), GL_STATIC_DRAW);

  // Binding edge buffer
  glGenBuffers(1, &edge_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edge_buffer);
  // Sending data to GPU
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      static_cast<GLsizeiptr>(m.edgeCount() * sizeof(Mesh::Edge)),
      m.edgeArray(),
      GL_STATIC_DRAW);

  // Binding face buffer
  glGenBuffers(1, &face_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);
  // Sending data to GPU
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      static_cast<GLsizeiptr>(m.faceCount() * sizeof(Mesh::Face)),
      m.faceArray(),
      GL_STATIC_DRAW);

  // Getting the attribute for the VAOs
  GLint position_attribute = glGetAttribLocation(program, "position");

  /// Creating the VAO for the edges ///
  glGenVertexArrays(1, &vao_edges);

  // Recording start
  glBindVertexArray(vao_edges);

  // Binding the vertices, enabling attributes and binding the edges as elements
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glVertexAttribPointer(position_attribute, 4, GL_FLOAT, false, sizeof(glm::vec4), nullptr);
  glEnableVertexAttribArray(position_attribute);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edge_buffer);

  // Recording end
  glBindVertexArray(0);

  /// Creating the VAO for the faces ///
  glGenVertexArrays(1, &vao_faces);

  // Recording start
  glBindVertexArray(vao_faces);

  // Binding the vertices, enabling attributes and binding the edges as elements
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glVertexAttribPointer(position_attribute, 4, GL_FLOAT, false, sizeof(glm::vec4), nullptr);
  glEnableVertexAttribArray(position_attribute);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);

  // Recording end
  glBindVertexArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void cs200::SolidRender::unloadMesh() {
  glDeleteVertexArrays(1, &vao_edges);
  glDeleteVertexArrays(1, &vao_faces);

  glDeleteBuffers(1, &face_buffer);
  glDeleteBuffers(1, &edge_buffer);
  glDeleteBuffers(1, &vertex_buffer);

  mesh_edge_count = 0;
  mesh_face_count = 0;
}

void cs200::SolidRender::displayEdges(const glm::vec4 &c) {
  glUseProgram(program);

  glUniform4fv(ucolor, 1, &c.r);
  glBindVertexArray(vao_edges);

  glDrawElements(GL_LINES, mesh_edge_count * 2, GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);
}

void cs200::SolidRender::displayFaces(const glm::vec4 &c) {
  glUseProgram(program);

  glUniform4fv(ucolor, 1, &c.r);
  glBindVertexArray(vao_faces);

  glDrawElements(GL_TRIANGLES, mesh_face_count * 3, GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);
}
