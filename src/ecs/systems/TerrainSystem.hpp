#pragma once

#include "../../core/Camera.hpp"

namespace ecs::TerrainSystem {

void init(entt::registry& registry, float planetRadius);
void update(entt::registry& registry);
void prerender(entt::registry& registry, core::Camera* activeCam, vec3 activeCamPos);
void reload(entt::registry& registry);

} // namespace TerrainSystem

