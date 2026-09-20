#pragma once

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"

namespace core {

struct Camera {
  float nearPlane = 0.1f;
  float farPlane = 1000.f;
  float fov = glm::radians(45.f);
  float yaw = glm::radians(-90.f);
  float pitch = glm::radians(0.f);
  float sensitivity = 1.f;
  vec3 position{};
  vec3 up{0.f, 1.f, 0.f};
  vec3 orientation{0.f, 0.f, -1.f};
  mat4 cachedProj{1.f};
  mat4 cachedView{1.f};

  void update(float aspectRatio) {
    orientation = normalize(vec3{
      cos(yaw) * cos(pitch),
      sin(pitch),
      sin(yaw) * cos(pitch)
    });

    cachedProj = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
    cachedView = glm::lookAt(position, position + orientation, up);
  }

  mat4 getLocalView(vec3 eye) {
    return glm::lookAt(eye, eye + orientation, up);
  }
};

} // namespace core

