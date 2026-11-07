#include "CameraSystem.hpp"

#include "entt/entity/fwd.hpp"
#include "../../core/EngineContext.hpp"
#include "../../core/ActiveCamera.hpp"
#include "../../gfx/AssetManager.hpp"
#include "../components/CameraComponent.hpp"
#include "../components/InputComponent.hpp"

namespace ecs::system::CameraSystem {

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

// Update the default camera
void updateDefault(entt::registry& registry) {
  const auto& ctx = registry.ctx().get<core::EngineContext>();
  const float aspectRatio = ctx.getAspectRatio_WidthOverHeight();
  auto& assetManager = registry.ctx().get<gfx::AssetManager>();

  auto* defaultCam = assetManager.getCamera("Default");

  for (auto entity : registry.view<CameraComponent, InputComponent>()) {
    auto& camComponent = registry.get<CameraComponent>(entity);
    auto& inputComponent = registry.get<InputComponent>(entity);

    if (camComponent.cam == defaultCam) {
      float upMask        = inputComponent.keyboardKeys[GLFW_KEY_SPACE];
      float downMask      = inputComponent.keyboardKeys[GLFW_KEY_LEFT_CONTROL];
      float leftMask      = inputComponent.keyboardKeys[GLFW_KEY_A];
      float rightMask     = inputComponent.keyboardKeys[GLFW_KEY_D];
      float forwardMask   = inputComponent.keyboardKeys[GLFW_KEY_W];
      float backMask      = inputComponent.keyboardKeys[GLFW_KEY_S];
      float shiftMask     = inputComponent.keyboardKeys[GLFW_KEY_LEFT_SHIFT];

      vec3 forward = defaultCam->orientation;
      vec3 up = defaultCam->up;
      vec3 right = glm::normalize(glm::cross(forward, up));
      float mult = inputComponent.shiftMultiplier * shiftMask + 1.f * (1.f - shiftMask);
      mult *= 1e3f;

      defaultCam->position +=  forward * mult * forwardMask;
      defaultCam->position += -right   * mult * leftMask;
      defaultCam->position += -forward * mult * backMask;
      defaultCam->position +=  right   * mult * rightMask;
      defaultCam->position +=  up      * mult * upMask;
      defaultCam->position += -up      * mult * downMask;

      defaultCam->update(aspectRatio);
      break;
    }
  }
}

}; // namespace ecs::CameraSystem

