#pragma once

#include "glm/gtc/quaternion.hpp"

namespace ecs::component {

struct JetPartComponent {
  float massWeight;
  vec3 offset;
  vec3 localBoxMin;
  vec3 localBoxMax;
  vec3 color{0.24377f, 0.355047f, 0.6226415f};
  glm::quat localRotation{1.f, 0.f, 0.f, 0.f};
  mat4 modelRelative{1.f};
};

} // namespace ecs::component::JetPartComponent

