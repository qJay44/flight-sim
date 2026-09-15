#pragma once

#include "../components/VelocityComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../../core/EngineContext.hpp"

namespace ecs::MovementSystem {

using namespace ecs::component;

inline void update(entt::registry& registry) {
  auto& ctx = registry.ctx().get<core::EngineContext>();
  float dt = ctx.dt;

  auto velView = registry.view<VelocityComponent, TransformComponent>();

  for (auto entity : velView) {
    auto& velComponent = registry.get<VelocityComponent>(entity);
    auto& transComponent = registry.get<TransformComponent>(entity);

    transComponent.pos += velComponent.velocity * velComponent.scale * dt;
    velComponent.velocity = vec3(0.f);
  }
}

}

