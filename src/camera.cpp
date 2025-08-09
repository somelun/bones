#include "camera.h"

#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

Camera::Camera(int width, int heigth, glm::vec3 position) {
  Camera::width = width;
  Camera::height = heigth;
  Camera::position = position;

  //
}

void Camera::Matrix(float FOV, float near_plane, float far_plane, GLuint shaderProgram, const char* uniform) {
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(1.0f);

  view = glm::lookAt(position, position + orientation, up);
  projection = glm::perspective(glm::radians(FOV), (float)(width / height), near_plane, far_plane);
}
