#pragma once

#include "entt/entity/fwd.hpp"

namespace ecs::CameraSystem {

void onMouseMove(entt::registry& registry, dvec2 mousePos);
void update(entt::registry& registry);

};

