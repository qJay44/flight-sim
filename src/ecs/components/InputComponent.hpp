#pragma once

namespace ecs::component {

struct InputComponent {
  bool disabled = false;
  float shiftMultiplier = 10.f;
};

} // namespace ecs::component

