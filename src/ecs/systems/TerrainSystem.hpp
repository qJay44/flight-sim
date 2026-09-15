#pragma once

#include "../../core/Camera.hpp"

namespace ecs::TerrainSystem {

constexpr int TERRAIN_MAX_NODES = 512;

void init(entt::registry& registry, float planetRadius);
void update(entt::registry& registry);
void render(entt::registry& registry, core::Camera* activeCam, vec3 activeCamPos);
void reload(entt::registry& registry);

} // namespace TerrainSystem

