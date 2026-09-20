#include "RenderSystem.hpp"

#include "../../core/EngineContext.hpp"
#include "../../core/ActiveCamera.hpp"
#include "../../gfx/Renderer.hpp"
#include "../../gfx/AssetManager.hpp"
#include "../../ecs/components/MeshComponent.hpp"
#include "../../ecs/components/AuxiliaryComponent.hpp"
#include "TransformSystem.hpp"
#include "TerrainSystem.hpp"

namespace ecs::RenderSystem {

using namespace component;

void render(entt::registry& registry) {
  auto& ctx = registry.ctx().get<core::EngineContext>();
  auto& renderer = registry.ctx().get<gfx::Renderer>();
  auto& assetManager = registry.ctx().get<gfx::AssetManager>();
  const auto& activeCam = registry.ctx().get<core::ActiveCamera>();

  gfx::Light* globalLight = assetManager.getLight("GlobalLight");
  assert(globalLight);

  renderer.newFrame(ctx.getWinSize());

  renderer.setGlobalLight(globalLight);

  TerrainSystem::render(registry, renderer);

  // Other stuff
  for (auto entity : registry.view<MeshComponent, TransformComponent, AuxiliaryComponent>()) {
    auto& meshComponent = registry.get<MeshComponent>(entity);
    auto& transComponent = registry.get<TransformComponent>(entity);

    gfx::Renderer::RenderCommand renderCmd{
      .shader = meshComponent.shader,
      .mesh = meshComponent.mesh,
    };

    meshComponent.shader->setUniformMatrix4f("u_model", TransformSystem::getModel(transComponent));
    meshComponent.shader->setUniformMatrix4f("u_proj", activeCam.cam->cachedProj);
    meshComponent.shader->setUniformMatrix4f("u_view", activeCam.cam->cachedView);

    renderer.submit(std::move(renderCmd));
  }

  renderer.renderFrame();
}

}; // namespace RenderSystem

