#pragma once

#include "../../core/Camera.hpp"

namespace ecs::component {

struct CameraComponent {
  core::Camera* cam;
};

} // namespace ecs::component

