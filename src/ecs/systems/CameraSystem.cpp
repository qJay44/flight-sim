#include "CameraSystem.hpp"

#include "entt/entity/fwd.hpp"
#include "../../core/EngineContext.hpp"
#include "../../core/ActiveCamera.hpp"
#include "../components/CameraComponent.hpp"
#include "../components/TransformComponent.hpp"

namespace ecs::CameraSystem {

using namespace ecs::component;

void onMouseMove(entt::registry& registry, dvec2 mousePos) {
  const auto& ctx = registry.ctx().get<core::EngineContext>();
  auto& activeCam = registry.ctx().get<core::ActiveCamera>();

  assert(activeCam.cam);

  dvec2 winSize = ctx.getWinSize();
  dvec2 winCenter = winSize * 0.5;
  dvec2 distFromCenter = mousePos - winCenter;

  dvec2 delta = dvec2(activeCam.cam->sensitivity) * distFromCenter / winCenter;
  activeCam.cam->yaw += delta.x;
  activeCam.cam->pitch = glm::clamp(activeCam.cam->pitch -= delta.y, -PI_2 + 0.1f, PI_2 - 0.1f);
}

void update(entt::registry& registry) {
  const auto& ctx = registry.ctx().get<core::EngineContext>();
  const float aspectRatio = ctx.getAspectRatio_WidthOverHeight();
  const auto& activeCam = registry.ctx().get<core::ActiveCamera>();

  for (auto entity : registry.view<CameraComponent>()) {
    auto& camComponent = registry.get<CameraComponent>(entity);

    if (camComponent.isDetached) {
      auto& transComponent = registry.get<TransformComponent>(entity);
      camComponent.cam->position = transComponent.pos;
    }

    camComponent.cam->update(aspectRatio);
    camComponent.isActive = camComponent.cam == activeCam.cam;
  }
}

}; // namespace ecs::CameraSystem

