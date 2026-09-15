#pragma once

#include "../../core/EngineContext.hpp"

namespace ecs::TimeSystem {
  [[nodiscard]]
  inline bool update(entt::registry& registry) {
    static double titleTimer = glfwGetTime();
    static double prevTime = titleTimer;
    static double currTime = prevTime + 1e6;

    auto& ctx = registry.ctx().get<core::EngineContext>();

    currTime = glfwGetTime();
    double dt = currTime - prevTime;

    // FPS cap
    if (dt < 1. / 90.)
      return false;

    prevTime = currTime;
    ctx.dt = dt;
    ctx.time += ctx.dt;

    return true;
  }
};

