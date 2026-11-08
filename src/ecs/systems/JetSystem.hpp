#pragma once

namespace ecs::system::JetSystem {

void init(entt::registry& registry);
void update(entt::registry& registry, float dt);
void render(entt::registry& registry, float dt);

}; // namespace ecs::system::JetSystem

