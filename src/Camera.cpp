/**
 * Name: Kishan S Patel
 * Email: kishan.patel@digipen.edu
 * Assignment Number: 4
 * Course: CS200
 * Term: Fall 2024
 *
 * File: Camera.cpp
 *
 */
#include "Camera.h"

glm::mat4 cs200::affineInverse(const glm::mat4 &A) {
  const float determinant = 1.f / (A[0][0] * A[1][1] - A[0][1] * A[1][0]);

  const glm::mat4 mat = {
      {A[1][1] * determinant, -A[0][1] * determinant, 0.f, 0.f},
      {-A[1][0] * determinant, A[0][0] * determinant, 0.f, 0.f},
      {0.f, 0.f, 1.f, 0.f},
      {0.f, 0.f, 0.f, 1.f} //
  };

  return mat * translate(-A[3]);
}

cs200::Camera::Camera(
    const glm::vec4 &C, const glm::vec4 &v, const float W, const float H) :
    center_point{C}, right_vector{}, up_vector{normalize(v)}, rect_width{W},
    rect_height{H} {
  right_vector = vector(-up_vector.y, up_vector.x);
}

cs200::Camera::Camera() : Camera{{0.f, 0.f, 0.f, 1.f}, {0, 1, 0, 1}, 2.f, 2.f} {}

cs200::Camera &cs200::Camera::moveRight(const float x) {
  center_point += right_vector * x;
  return *this;
}
cs200::Camera &cs200::Camera::moveUp(const float y) {
  center_point += up_vector * y;
  return *this;
}

cs200::Camera &cs200::Camera::rotate(const float t) {
  const glm::mat4 rotation{cs200::rotate(t)};

  right_vector = rotation * right_vector;
  up_vector = rotation * up_vector;

  return *this;
}

cs200::Camera &cs200::Camera::zoom(const float f) {
  rect_width *= f;
  rect_height *= f;
  return *this;
}

glm::mat4 cs200::cameraToWorld(const Camera &cam) {
  return glm::mat4{cam.right(), cam.up(), {0.f, 0.f, 1.f, 0.f}, cam.center()};
}

glm::mat4 cs200::worldToCamera(const Camera &cam) {
  return affineInverse(cameraToWorld(cam));
}

glm::mat4 cs200::cameraToNDC(const Camera &cam) {
  return scale(1.f / cam.width(), 1.f / cam.height());
}

glm::mat4 cs200::NDCToCamera(const Camera &cam) {
  return scale(cam.width(), cam.height());
}
