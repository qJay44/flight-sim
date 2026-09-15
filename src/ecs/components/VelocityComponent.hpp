#pragma once

namespace ecs::component {

struct VelocityComponent {
  vec3 velocity;
  float scale{1.f};
};

} // namespace ecs::component

