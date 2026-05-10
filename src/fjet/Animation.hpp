#pragma once

#include "glm/ext/quaternion_trigonometric.hpp"

struct Animation {
  float maxAngleRad;
  float minAngleRad;
  float rotSpeedRad;
  vec3 rotAxis;

  float currentAngleRad = 0.f;

  [[nodiscard]]
  glm::quat rot(float rotDir, float dt) {
    currentAngleRad += rotSpeedRad * rotDir * dt;
    currentAngleRad = std::clamp(currentAngleRad, minAngleRad, maxAngleRad);

    return glm::angleAxis(currentAngleRad, rotAxis);
  }

  void rotBack(float rotDir, float scale) {
    currentAngleRad -= rotSpeedRad * rotDir * scale + currentAngleRad * scale;
  }
};

