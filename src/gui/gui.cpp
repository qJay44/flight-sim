#include "gui.hpp"

#include "imgui.h"
// #include "implot.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ProfilerManager.hpp"
#include "components.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../gfx/AssetManager.hpp"
#include "../gfx/terrain/GenerationManager.hpp"
#include "../core/ActiveCamera.hpp"
#include "../ecs/systems/TerrainSystem.hpp"
#include "LoaderWidget.hpp"

namespace gui {

namespace {

bool configCollapsed = true;
bool infoCollapsed = true;

} // namespace

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
  ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
}

void init(GLFWwindow* window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  // ImPlot::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init();
}

void toggleConfig() { configCollapsed = !configCollapsed;}
void toggleInfo()  { infoCollapsed = !infoCollapsed; }

void render(entt::registry& registry) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  auto& assetManager = registry.ctx().get<gfx::AssetManager>();
  auto& profiler = registry.ctx().get<ProfilerManager>();
  auto& activeCam = registry.ctx().get<core::ActiveCamera>();

  // ::::: Config window ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

  ImGui::SetNextWindowPos({0, 0}, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowCollapsed(configCollapsed);

  ImGui::Begin("Config");

  if (ImGui::CollapsingHeader("Global light")) {
    auto* light = assetManager.getLight("GlobalLight");
    ImGui::SliderFloat("Ambient", &light->ambient, 0.f, 1.f);
    ImGui::SliderFloat("Specular", &light->specular, 0.f, 1.f);
    ImGui::ColorEdit3("Color", glm::value_ptr(light->color));
    ImGui::SliderFloat("Intensity", &light->intensity, 0.f, 20.f);
    ImGui::SliderFloat("Focus", &light->focus, 0.f, 1000.f);
  }

  if (ImGui::CollapsingHeader("Cameras")) {
    auto camView = registry.view<CameraComponent, TransformComponent>();
    auto& activeCam = registry.ctx().get<core::ActiveCamera>();

    for (auto entity : camView) {
      size_t id = static_cast<size_t>(entity);
      auto& camComponent = registry.get<CameraComponent>(entity);

      ImGui::PushID(id);
      ImGui::SetNextItemOpen(camComponent.cam == activeCam.cam, ImGuiCond_FirstUseEver);

      if (ImGui::TreeNode(std::format("#{}", id).c_str())) {
        auto& transComponent = registry.get<TransformComponent>(entity);

        drawCameraComponent(camComponent, activeCam);
        drawTransformComponent(transComponent);

        ImGui::TreePop();
      }

      ImGui::PopID();
    }
  }

  if (ImGui::CollapsingHeader("Terrain")) {
    auto& gm = registry.ctx().get<gfx::terrain::GenerationManager>();
    auto& cfg = gm.getConfig();
    vec3 dirToCenter = glm::normalize(activeCam.cam->position);

    ImGui::Text("Height scale: [%.2f]", cfg.planetRadius * cfg.planetRadiusPercent);
    ImGui::Text("Free   slots: [%zu]", gm.getFreeSlots());
    ImGui::Text("Cached slots: [%zu]", gm.getCachedSlots());
    ImGui::Text("Altitude: [%.2f]", glm::distance(activeCam.cam->position, cfg.planetRadius * dirToCenter));

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
      u |= ImGui::SliderFloat("Start amplitude (detail)", &cfg.initAmplitudeDetail, 0.f, 1.f);
      u |= ImGui::SliderFloat("Start frequency (detail)", &cfg.initFrequencyDetail, 0.f, 10.f);
      u |= ImGui::SliderFloat("Amplitude gain (detail)", &cfg.gainDetail, 0.f, 1.f);
      u |= ImGui::SliderFloat("Lacunarity (detail)", &cfg.lacunarityDetail, 1.f, 10.f);
      u |= ImGui::SliderFloat("F1 Voroni frequency", &cfg.f1VoronoiFreq, 0.f, 100.f);
      u |= ImGui::SliderFloat("Displace strength", &cfg.displaceStrength, 0.f, 10.f);
      u |= ImGui::SliderFloat("Continent frequency", &cfg.continentFreq, 0.f, 15.f);
      u |= ImGui::SliderFloat("F2-F1 Voronoi frequency", &cfg.f2f1VoronoiFreq, 0.f, 10.f);
      u |= ImGui::SliderInt("Octaves", &cfg.octavesDisplace, 1, 10);
      u |= ImGui::SliderInt("Terrace steps", &cfg.terraceSteps, 1, 30);

      ImGui::SeparatorText("Load/Save");
      static LoaderWidget lw("heightmap1");
      u |= lw.render(cfg);

      if (u)
        ecs::TerrainSystem::reload(registry);

      ImGui::SeparatorText("Lands");
      for (auto entity : registry.view<terrain::TerrainComponent, MeshComponent>()) {
        size_t id = static_cast<size_t>(entity);
        ImGui::PushID(id);
        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

        if (ImGui::TreeNode(std::format("#{}", id).c_str())) {
          auto& terrainComponent = registry.get<terrain::TerrainComponent>(entity);
          auto& meshComponent = registry.get<MeshComponent>(entity);

          ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
          drawTerrainComponent(terrainComponent);
          drawMeshComponent(meshComponent);

          ImGui::TreePop();
        }

        ImGui::PopID();
      }

      ImGui::SeparatorText("Waters");
      for (auto entity : registry.view<terrain::WaterComponent, MeshComponent>()) {
        size_t id = static_cast<size_t>(entity);
        ImGui::PushID(id);
        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

        if (ImGui::TreeNode(std::format("#{}", id).c_str())) {
          auto& waterComponent = registry.get<terrain::WaterComponent>(entity);
          auto& meshComponent = registry.get<MeshComponent>(entity);

          ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
          drawWaterComponent(waterComponent);
          drawMeshComponent(meshComponent);

          ImGui::TreePop();
        }

        ImGui::PopID();
      }
    }
  }

  ImGui::End();

  // ::::: Info window ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImVec2 posBR = viewport->WorkPos + viewport->WorkSize;

  ImGui::SetNextWindowPos(posBR, ImGuiCond_Always, {1.f, 1.f});
  ImGui::SetNextWindowCollapsed(infoCollapsed);

  profiler.renderTasks();

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  // ImPlot::DestroyContext();
  ImGui::DestroyContext();
}

} // namespace gui

