#pragma once

#include "../../core/Camera.hpp"

namespace ecs::component {

struct CameraComponent {
  core::Camera* cam;
  bool isActive = false;
  bool isDetached = false; // NOTE: Make sure whatever changes this field adds/removes TransformComponent
};

} // namespace ecs::component

