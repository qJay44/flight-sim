#include "RenderSystem.hpp"

#include "../components/MeshComponent.hpp"
#include "../components/CameraComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/TextureComponent.hpp"
#include "../../core/EngineContext.hpp"
#include "../../gfx/Renderer.hpp"
#include "../../gfx/AssetManager.hpp"
#include "TransformSystem.hpp"
#include "TerrainSystem.hpp"

namespace ecs::RenderSystem {

using namespace ecs::component;

void render(entt::registry& registry) {
  auto& ctx = registry.ctx().get<core::EngineContext>();
  auto& renderer = registry.ctx().get<gfx::Renderer>();
  auto& assetManager = registry.ctx().get<gfx::AssetManager>();

  auto camView = registry.view<CameraComponent, TransformComponent>();

  [[maybe_unused]] core::Camera* activeCam = nullptr;
  vec3 activeCamPos{};
  for (auto entity : camView) {
    const auto& camComponent = registry.get<CameraComponent>(entity);
    if (camComponent.isActive) {
      const auto& transComponent = registry.get<TransformComponent>(entity);

      activeCam = camComponent.cam;
      activeCamPos = transComponent.pos;
      break;
    }
  }

  gfx::Light* globalLight = assetManager.getLight("GlobalLight");

  assert(activeCam);
  assert(globalLight);

  TerrainSystem::prerender(registry, activeCam, activeCamPos);

  auto meshView = registry.view<MeshComponent, TransformComponent>();

  renderer.beginFrame(ctx.getWinSize());
  renderer.setProjectionMat(activeCam->cachedProj);
  renderer.setViewMat(activeCam->cachedView);
  renderer.setGlobalLight(globalLight);

  for (auto& entity : meshView) {
    const auto& meshComponent = registry.get<MeshComponent>(entity);
    const auto& transComponent = registry.get<TransformComponent>(entity);
    const auto* textureComponentPtr = registry.try_get<TextureComponent>(entity);

    if (meshComponent.disabled)
      continue;

    gfx::Renderer::RenderCommand renderCmd{
      .shader = meshComponent.shader,
      .mesh = meshComponent.mesh,
      .cam  = activeCam,
      .camPos = activeCamPos,
      .model = TransformSystem::getModel(transComponent),
      .textures = textureComponentPtr ? textureComponentPtr->textures : std::vector<gfx::Texture*>{}
    };

    renderer.submit(std::move(renderCmd));
  }

  renderer.endFrame(ctx);
}

}; // namespace RenderSystem

