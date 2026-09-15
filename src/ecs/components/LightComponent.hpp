#pragma once

#include "../../core/Light.hpp"

namespace ecs::component {

struct LightComponent {
  core::Light* light;
};

} // namespace ecs::component

