#pragma once

#include "glm/gtc/quaternion.hpp"

namespace ecs::component {

struct TransformComponent {
  vec3 pos{0.f};
  glm::quat rotation{1.f, 0.f, 0.f, 0.f};
  vec3 scale{1.f};
};

} // namespace ecs::component

