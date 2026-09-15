#include "CameraSystem.hpp"

#include "entt/entity/fwd.hpp"
#include "../../core/EngineContext.hpp"
#include "../components/CameraComponent.hpp"
#include "../components/TransformComponent.hpp"

namespace ecs::CameraSystem {

using namespace ecs::component;

void onMouseMove(entt::registry& registry, dvec2 mousePos) {
  const auto& ctx = registry.ctx().get<core::EngineContext>();
  auto camView = registry.view<CameraComponent, TransformComponent>();

  core::Camera* activeCam = nullptr;
  for (auto entity : camView) {
    const auto& camComponent = registry.get<CameraComponent>(entity);
    if (camComponent.isActive) {
      activeCam = camComponent.cam;
      break;
    }
  }

  assert(activeCam);

  dvec2 winSize = ctx.getWinSize();
  dvec2 winCenter = winSize * 0.5;
  dvec2 distFromCenter = mousePos - winCenter;

  dvec2 delta = dvec2(activeCam->sensitivity) * distFromCenter / winCenter;
  activeCam->yaw += delta.x;
  activeCam->pitch = glm::clamp(activeCam->pitch -= delta.y, -PI_2 + 0.1f, PI_2 - 0.1f);
}

void update(entt::registry& registry) {
  auto view = registry.view<CameraComponent, TransformComponent>();
  const auto& ctx = registry.ctx().get<core::EngineContext>();
  const float aspectRatio = ctx.getAspectRatio_WidthOverHeight();

  for (auto entity : view) {
    auto& cam = registry.get<CameraComponent>(entity);
    if (cam.isActive) {
      const auto& transform = registry.get<TransformComponent>(entity);
      cam.cam->update(aspectRatio, transform.pos);
      break;
    }
  }
}

}; // namespace ecs::CameraSystem

