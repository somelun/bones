#pragma once

#include "glm/glm.hpp"
#include <GL/glew.h>

class Camera {
public:
  Camera(int width, int heigth, glm::vec3 position);

  glm::vec3 position;
  glm::vec3 orientation = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

  int width;
  int height;

  void Matrix(float FOV, float near_plane, float far_plane, GLuint shaderProgram, const char* uniform);
};
