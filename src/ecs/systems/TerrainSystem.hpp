#pragma once

namespace ecs::TerrainSystem {

void init(entt::registry& registry);
void update(entt::registry& registry);
void render(entt::registry& registry);
void reload(entt::registry& registry);

} // namespace TerrainSystem

