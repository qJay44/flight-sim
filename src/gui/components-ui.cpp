#include "components-ui.hpp"

#include "imgui.h"

#include "../ecs/components/CameraComponent.hpp"
#include "../ecs/components/TransformComponent.hpp"
#include "../ecs/components/TerrainComponent.hpp"
#include "../ecs/components/VelocityComponent.hpp"
#include "../ecs/systems/TerrainSystem.hpp"
#include "../gfx/terrain/GenerationManager.hpp"
#include "LoaderWidget.hpp"

namespace gui {

using namespace ecs::component;

void drawCameraUi(entt::registry& registry) {
  if (ImGui::CollapsingHeader("Cameras")) {
    auto camView = registry.view<CameraComponent, TransformComponent>();

    for (auto entity : camView) {
      auto* camComponent = &registry.get<CameraComponent>(entity);
      auto* camPos = &registry.get<TransformComponent>(entity).pos;
      auto& velComponent = registry.get<VelocityComponent>(entity);

      if (ImGui::TreeNode(&entity, "#%zu%s", (size_t)entity, camComponent->isActive ? " (Active)" : "")) {
        auto* cam = camComponent->cam;

        ImGui::Text("Up: [%.2f, %.2f, %2.f]", cam->up.x, cam->up.y, cam->up.z);
        ImGui::Text("Orientation: [%.2f, %.2f, %2.f]", cam->orientation.x, cam->orientation.y, cam->orientation.z);

        ImGui::SliderFloat("Near", &cam->nearPlane, 0.01f, 10.f);
        ImGui::DragFloat("Far", &cam->farPlane, 1.f, 1e5f);
        ImGui::SliderAngle("Fov", &cam->fov);
        ImGui::SliderAngle("Yaw", &cam->yaw);
        ImGui::SliderAngle("Pitch", &cam->pitch);
        ImGui::SliderFloat("Sensitivity", &cam->sensitivity, 0.1f, 10.f);
        ImGui::DragFloat3("Position", (float*)camPos);
        ImGui::SliderFloat("Velocity scale", &velComponent.scale, 0.1f, 1e4f);

        if (ImGui::Checkbox("Is active", &camComponent->isActive)) {
          for (auto otherEntity : camView) {
            auto& otherCamComponent = registry.get<CameraComponent>(otherEntity);

            if (camComponent->cam != otherCamComponent.cam) {
              otherCamComponent.isActive = !camComponent->isActive;
            } else {
              auto& transComponent = registry.get<TransformComponent>(otherEntity);
              camPos = &transComponent.pos;
            }
          }
        }

        ImGui::TreePop();
      }
    }
  }
}

void drawTerrainUi(entt::registry& registry) {
  if (ImGui::CollapsingHeader("Terrain")) {
    for (auto entity : registry.view<terrain::TerrainComponent>()) {
      auto& terrain = registry.get<terrain::TerrainComponent>(entity);
      auto& gm = registry.ctx().get<gfx::terrain::GenerationManager>();
      auto& cfg = gm.getConfig();

      float nodesUsed = static_cast<float>(terrain.activeLeafs) / TERRAIN_MAX_NODES;
      vec4 nodesColor = nodesUsed > 0.8f ? vec4(1.f, 0.f, 0.f, 1.f) : nodesUsed > 0.5f ? vec4(1.f, 1.f, 0.f, 1.f) : vec4(1.f);

      ImGui::TextColored(nodesColor, "Active leafs: [%zu] / [%d]", terrain.activeLeafs, TERRAIN_MAX_NODES);
      ImGui::Text("Height scale: [%.2f]", cfg.planetRadius * cfg.planetRadiusPercent);

      ImGui::Separator();

      ImGui::SliderFloat("Sea threshold", &terrain.seaThreshold, 0.f, 1.f);
      ImGui::SliderFloat("Sand threshold", &terrain.sandThreshold, 0.f, 1.f);
      ImGui::SliderFloat("Moutain threshold", &terrain.mountainThreshold, 0.f, 1.f);
      ImGui::DragFloat("Wave scale", &terrain.waveScale);
      ImGui::DragFloat("Wave raidus", &terrain.waterRadiusScale);
      ImGui::DragFloat("Foam sharpness", &terrain.foamSharpness);
      ImGui::SliderInt("Quadtree max depth", &terrain.qtMaxDepth, 1, 20);
      ImGui::SliderFloat("Quadtree split threshold", &terrain.qtSplitThreshold, 0.f, 1.f);

      ImGui::SeparatorText("FBM");
      {
        bool u = false;

        u |= ImGui::DragFloat("Planet raidus", &cfg.planetRadius);
        u |= ImGui::SliderFloat("Planet radius percent (height scale)", &cfg.planetRadiusPercent, 0.f, 1.f);
        u |= ImGui::SliderFloat("Global scale", &cfg.globalScale, 1.f, 50.f);
        u |= ImGui::SliderFloat("Start amplitude", &cfg.initAmplitude, 0.f, 1.f);
        u |= ImGui::SliderFloat("Start frequency", &cfg.initFrequency, 0.f, 10.f);
        u |= ImGui::SliderFloat("Amplitude gain", &cfg.gain, 0.f, 1.f);
        u |= ImGui::SliderFloat("Lacunarity", &cfg.lacunarity, 1.f, 10.f);
        u |= ImGui::SliderFloat("F1 Voroni frequency", &cfg.f1VoronoiFreq, 0.f, 100.f);
        u |= ImGui::SliderFloat("Displace strength", &cfg.displaceStrength, 0.f, 50.f);
        u |= ImGui::SliderFloat("Continent frequency", &cfg.continentFreq, 0.f, 50.f);
        u |= ImGui::SliderFloat("F2-F1 Voronoi frequency", &cfg.f2f1VoronoiFreq, 0.f, 100.f);
        u |= ImGui::SliderInt("Octaves", &cfg.octaves, 1, 10);
        u |= ImGui::SliderInt("Terrace steps", &cfg.terraceSteps, 1, 30);

        ImGui::SeparatorText("Load/Save");
        static LoaderWidget lw("heightmap1");
        if (lw.render(cfg))
          u = true;

        if (u)
          ecs::TerrainSystem::reload(registry);
      }
    }
  }
}

} // namespace gui

