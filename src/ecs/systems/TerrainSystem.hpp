#pragma once

#include "../../gfx/Renderer.hpp"

namespace ecs::TerrainSystem {

void init(entt::registry& registry);
void update(entt::registry& registry);
void render(entt::registry& registry, gfx::Renderer& renderer);
void reload(entt::registry& registry);

} // namespace TerrainSystem

