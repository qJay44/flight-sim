#pragma once

namespace ecs::component {

struct InputComponent {
  bool disabled = false;
  float shiftMultiplier = 10.f;
  std::array<bool, GLFW_KEY_LAST> keyboardKeys{};
};

} // namespace ecs::component

