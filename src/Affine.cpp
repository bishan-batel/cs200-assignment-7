// /**
//  * Name: Kishan S Patel
//  * Email: kishan.patel@digipen.edu
//  * Assignment Number: 1
//  * Course: CS200
//  * Term: Fall 2024
//  *
//  * File: Affine.h
//  *
//  *  2D Affine Transformation Functions
//  */
//
// #include "Affine.h"
//
// namespace cs200 {
//   bool isPoint(const glm::vec4 &p) { return near(p.z, 0.f) and near(p.w, 1.0f); }
//
//   bool isVector(const glm::vec4 &v) { return near(v.z, 0.f) and near(v.w, 0.0f); }
//
//   bool isAffine(const glm::mat4 &A) {
//     return //
//         isVector(A[0]) and // first column must be  [x, y, 0, 0]
//         isVector(A[1]) and // second column must be [x, y, 0, 0]
//
//         near(A[2].x, 0.f) and // third column must be [0, 0, 1, 0]
//         near(A[2].y, 0.f) and //
//         near(A[2].z, 1.f) and //
//         near(A[2].w, 0.f) and //
//
//         not near(A[3].x, 0.f) and // last column must be [x,y,0,1], x != 0, y != 0
//         not near(A[3].y, 0.f) and //
//         near(A[3].z, 0.f) and // should be 0
//         near(A[3].w, 1.f); // last component should be 1.0
//   }
//
//   glm::vec4 point(const float x, const float y) { return {x, y, 0.0f, 1.0f}; }
//
//   glm::vec4 vector(const float x, const float y) { return {x, y, 0.0f, 0.0f}; }
//
//   glm::mat4 affine(const glm::vec4 &u, const glm::vec4 &v, const glm::vec4 &C) {
//     return {
//         {u.x, u.y, 0.0f, 0.0f},
//         {v.x, v.y, 0.0f, 0.0f},
//         {0.0f, 0.0f, 1.0f, 0.0f},
//         {C.x, C.y, 0.0f, 1.0f},
//     };
//   }
//   glm::mat4 rotate(const float t) {
//     const float theta = glm::radians(t);
//
//     const glm::mat4 output{
//         {std::cos(theta), std::sin(theta), 0.0f, 0.0f},
//         {-std::sin(theta), std::cos(theta), 0.0f, 0.0f},
//         {0.0f, 0.0f, 1.0f, 0.0f},
//         {0.0f, 0.0f, 0.0f, 1.0f},
//     };
//     //
//     return {};
//   }
//
//   glm::mat4 translate(const glm::vec4 &v) {
//     return {
//         {1.0f, 0.0f, 0.0f, 0.0f},
//         {0.0f, 1.0f, 0.0f, 0.0f},
//         {0.0f, 0.0f, 1.0f, 0.0f},
//         {v.x, v.y, 0.0f, 1.0f},
//     };
//   }
//
//   glm::mat4 scale(const float r) { return scale(r, r); }
//
//   glm::mat4 scale(const float rx, const float ry) {
//     return {
//         {rx, 0.0f, 0.0f, 0.0f},
//         {0.0f, ry, 0.0f, 0.0f},
//         {0.0f, 0.0f, 1.0f, 0.0f},
//         {0.0f, 0.0f, 0.0f, 1.0f},
//     };
//   }
// } // namespace cs200
/**
 * File: Affine.cpp
 * Name: Edgar Jose Donoso Mansilla (e.donosomansilla)
 * Assignment: 1
 * Course: CS200
 * Term: FALL24
 */

#include "Affine.h"

namespace cs200 {

  bool isPoint(const glm::vec4 &P) { return near(P.w, 1); }

  bool isVector(const glm::vec4 &v) { return near(v.w, 0); }

  bool isAffine(const glm::mat4 &A) {
    for (int i = 0; i < 4; i++) {
      if (i == 2) {
        if (!near(A[i][2], 1)) {
          return false;
        }
        if (!near(A[2][i], 1)) {
          return false;
        }
      } else {
        if (!near(A[i][2], 0)) {
          return false;
        }
        if (!near(A[2][i], 0)) {
          return false;
        }
      }

      if (i == 3) {
        if (!near(A[i][3], 1)) {
          return false;
        }
      } else {
        if (!near(A[i][3], 0)) {
          return false;
        }
      }
    }

    return !near(A[3][0], 0) && !near(A[3][1], 0);
  }

  glm::vec4 point(float x, float y) { return glm::vec4{x, y, 0, 1}; }

  glm::vec4 vector(float x, float y) { return glm::vec4{x, y, 0, 0}; }

  glm::mat4 affine(const glm::vec4 &u, const glm::vec4 &v, const glm::vec4 &C) {
    return {{u.x, u.y, 0, 0}, {v.x, v.y, 0, 0}, {0, 0, 1, 0}, {C.x, C.y, 0, 1}};
  }

  glm::mat4 rotate(float t) {

    float cosine = std::cos(glm::radians(t));
    float sine = std::sin(glm::radians(t));
    return {{cosine, sine, 0, 0}, {-sine, cosine, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
  }

  glm::mat4 translate(const glm::vec4 &v) { return {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {v.x, v.y, 0, 1}}; }

  glm::mat4 scale(float r) { return {{r, 0, 0, 0}, {0, r, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}; }

  glm::mat4 scale(float rx, float ry) { return {{rx, 0, 0, 0}, {0, ry, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}; }

} // namespace cs200
