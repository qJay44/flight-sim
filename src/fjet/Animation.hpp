#pragma once

#include "glm/ext/quaternion_trigonometric.hpp"

struct Animation {
  float maxAngleRad;
  float minAngleRad;
  float rotSpeedRad;
  vec3 rotAxis;

  float currentAngleRad = 0.f;
  glm::quat currentRotation;

  void rotate(float rotDir, float dt) {
    currentAngleRad += rotSpeedRad * rotDir * dt;
    currentAngleRad = std::clamp(currentAngleRad, minAngleRad, maxAngleRad);

    currentRotation = glm::angleAxis(currentAngleRad, rotAxis);
  }

  void rotateBack(float rotDir, float scale) {
    currentAngleRad -= rotSpeedRad * rotDir * scale + currentAngleRad * scale;
  }
};

