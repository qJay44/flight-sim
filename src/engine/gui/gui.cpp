#include "gui.hpp"

#include <cassert>

#include "imgui.h"
// #include "implot.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "glm/gtc/type_ptr.hpp"
#include "global.hpp"
#include "utils/utils.hpp"
#include "../../terrain/shared.hpp"

static bool configCollapsed = true;
static bool infoCollapsed = true;

Sun* gui::sunPtr = nullptr;
FighterJet* gui::fjetPtr = nullptr;
terrain::Terrain* gui::terrainPtr = nullptr;

u16 gui::fps = 1;

namespace {

ImFont* fontMain = nullptr;

void TextVec3(const char* label, const vec3& v) {
  ImGui::Text("%s: [%.2f, %.2f, %.2f]", label, v.x, v.y, v.z);
}

[[maybe_unused]]
void CreateMyTooltip(const char* text) {
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(global::getWinCenter().x * 0.5f);

    ImGui::Text("%s", text);

    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

} // namespace

void gui::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
}

void gui::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

void gui::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
  ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
}

void gui::init() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  // ImPlot::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

  fontMain = io.Fonts->AddFontFromFileTTF("res/fonts/FiraCodeNerdFontMono-Regular.ttf", 16.f);
  if (!fontMain)
    error("[gui::init] Font wasn't added");

  ImGui_ImplGlfw_InitForOpenGL(global::window, true);
  ImGui_ImplOpenGL3_Init();

  io.Fonts->Build();
}

void gui::toggleConfig() { configCollapsed = !configCollapsed; }
void gui::toggleInfo()   { infoCollapsed   = !infoCollapsed;   }

void gui::draw() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // ::::: Config window ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

  ImGui::SetNextWindowPos({0, 0}, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowCollapsed(configCollapsed);

  auto _task = global::profiler->startScopedTask("gui::draw");

  ImGui::Begin("Config");

  ImGui::Text("FPS: %d / %f.5 ms", fps, global::dt);

  // ===== F15 General =================================================================================== //

  assert(fjetPtr);
  if (ImGui::CollapsingHeader("F15 General")) {
    ImGui::Checkbox("Show HUD", &global::jetDrawHUD);
    ImGui::Checkbox("Show parts masses", &global::jetDrawDebugMass);
    ImGui::Checkbox("Show Hitboxes", &global::jetDrawDebugHitboxes);
  }

  // ===== F15 Body ====================================================================================== //

  if (ImGui::CollapsingHeader("F15 Body")) {
    auto& body = fjetPtr->body;
    ImGui::SliderFloat("Max thrust", &body.cfg.maxThrust, 0.f, 1e6f);
    TextVec3("Local Velocity", body.localVelocity);
    TextVec3("Local Angular Velocity", body.localAngularVelocity);
    ImGui::Text("Angle of attack     [%.2f]", body.angleOfAttack);
    ImGui::Text("Angle of attack yaw [%.2f]", body.angleOfAttackYaw);
    ImGui::Text("G [%.2f]", body.lastG);

    ImGui::Separator();
    ImGui::SliderFloat("Induced drag", &body.cfg.inducedDrag, 0.f, 100.f);
    ImGui::SliderFloat("Lift power", &body.cfg.liftPower, 0.f, 100.f);
    ImGui::SliderFloat("Rudder power", &body.cfg.rudderPower, 0.f, 100.f);
    ImGui::SliderFloat("Flaps lifst power", &body.cfg.flapsLiftPower, 0.f, 100.f);
    ImGui::SliderFloat("Flaps AOA bias", &body.cfg.flapsAOABias, 5.f, 15.f);

    ImGui::SeparatorText("Physics core");
    {
      auto& core = body.rigidbody;
      ImGui::Text("Mass: %.4f kg", core.mass);
      TextVec3("Position", core.position);
      TextVec3("Velocity", core.velocity);
      TextVec3("Angular Velocity", core.angularVelocity);
    }

    ImGui::SeparatorText("Center of mass");

    if (ImGui::TreeNode("Parts")) {
      for (AircraftPart* p : body.parts) {
        if (ImGui::TreeNode(p->name.c_str())) {
          ImGui::Text("Mass: %.2f kg", p->mass);
          ImGui::ColorEdit3("Debug color", glm::value_ptr(p->color));

          ImGui::TreePop();
        }
      }
      ImGui::TreePop();
    }
  }

  // ===== Terrain ======================================================================================= //

  assert(terrainPtr);
  if (ImGui::CollapsingHeader("Terrain")) {
    ImGui::SeparatorText("General");
    ImGui::SliderFloat("Planet radius", &terrain::planetRadius, 1.f, 1e6f);

    ImGui::SeparatorText("Height coloring thresholds");
    ImGui::SliderFloat("Sea", &terrainPtr->seaThreshold, 0.f, 1.f);
    ImGui::SliderFloat("Sand", &terrainPtr->sandThreshold, terrainPtr->seaThreshold, 1.f);
    ImGui::SliderFloat("Mountain", &terrainPtr->mountainThreshold, terrainPtr->sandThreshold, 1.f);

    ImGui::SeparatorText("FBM");
    {
      auto& cfg = terrain::Quadnode::gm.cfg;
      bool u = false;

      u |= ImGui::SliderFloat("Planet radius percent (height scale)", &terrainPtr->planetRadiusPercent, 0.f, 1.f);
      u |= ImGui::SliderFloat("Start amplitude", &cfg.initAmplitude, 0.f, 1.f);
      u |= ImGui::SliderFloat("Start frequency", &cfg.initFrequency, 0.f, 10.f);
      u |= ImGui::SliderFloat("Amplitude gain", &cfg.gain, 0.f, 1.f);
      u |= ImGui::SliderFloat("Lacunarity", &cfg.lacunarity, 1.f, 10.f);
      u |= ImGui::SliderFloat("Frequency multiplier", &cfg.frequencyMult, 1.f, 10.f);
      u |= ImGui::SliderFloat("Amplitude multiplier", &cfg.amplitudeMult, 1.f, 100.f);
      u |= ImGui::SliderFloat("Detail frequency multiplier", &cfg.detailFrequencyMult, 1.f, 10.f);
      u |= ImGui::SliderFloat("Detail amplitude multiplier", &cfg.detailAmplitudeMult, 1.f, 100.f);
      u |= ImGui::SliderInt("Octaves", &cfg.octaves, 1, 10);
      u |= ImGui::SliderInt("Detail octaves", &cfg.detailOctaves, 1, 10);

      if (u)
        terrainPtr->reload();
    }

    ImGui::Separator();
    ImGui::SliderFloat2("Cliff edges",                                 glm::value_ptr(terrainPtr->cliffEdges),  0.f, 1.f);
    ImGui::SliderFloat2("Dirt edges (inversed)",                       glm::value_ptr(terrainPtr->dirtEdges),   0.f, 1.f);
    ImGui::SliderFloat2("Snow edges",                                  glm::value_ptr(terrainPtr->snowEdges),   0.f, 1.f);
    ImGui::SliderFloat2("Sand edges (inversed) water height offset",   glm::value_ptr(terrainPtr->sandEdges),   0.f, 1.f);
    ImGui::SliderFloat2("Grass0 edges",                                glm::value_ptr(terrainPtr->grass0Edges), 0.f, 1.f);
    ImGui::SliderFloat2("Grass1 edges (inversed) grass height offset", glm::value_ptr(terrainPtr->grass1Edges), 0.f, 1.f);
    ImGui::SliderFloat2("Grass2 edges",                                glm::value_ptr(terrainPtr->grass2Edges), 0.f, 1.f);
  }

  if (ImGui::CollapsingHeader("Postprocess")) {
    ImGui::SliderFloat("Atmosphere scale", &terrainPtr->atmosphereScale, 0.f, 20.f);

    ImGui::SeparatorText("Water");
    ImGui::SliderFloat("Shore scale", &terrainPtr->waterShoreScale, 0.f, 2.f);
    ImGui::SliderFloat("Rerfraction scale", &terrainPtr->waterRefractionScale, 0.f, 2.f);
    ImGui::SliderFloat("Rerfraction distort scale", &terrainPtr->waterRefractionDistortScale, 0.f, 2.f);
    ImGui::SliderFloat("Normal UV scale", &terrainPtr->waterNormalScaleUV, 0.f, 20.f);
    ImGui::SliderFloat("Noise scale", &terrainPtr->waterNoiseScale, 0.f, 20.f);
    ImGui::SliderFloat("Foam edge0", &terrainPtr->foamEdge0, -10.f, 10.f);
    ImGui::SliderFloat("Foam edge1", &terrainPtr->foamEdge1, -10.f, 10.f);

    ImGui::SeparatorText("Fog");
    ImGui::SliderFloat("Density", &terrainPtr->fogDensity, 0.f, 10.f);
    ImGui::SliderFloat("Density falloff", &terrainPtr->fogDensityFalloff, 0.f, 0.001f);
    ImGui::SliderFloat("Horizon thickness", &terrainPtr->horizonThickness, 0.f, 80.f);
    ImGui::SliderFloat("Horizon falloff", &terrainPtr->horizonFalloff, 0.f, 20.f);
  }

  // ===== Active camera ================================================================================= //

  auto& cam = Camera::activeCam;
  assert(cam);
  if (ImGui::CollapsingHeader("Active camera")) {
    ImGui::SliderFloat("Near##2", &cam->nearPlane, 0.01f, 1.f);
    ImGui::SliderFloat("Far##2", &cam->farPlane,  10.f, 1000.f);
    ImGui::SliderFloat("Speed##2", &cam->speedDefault, 1.f, 50.f);
    ImGui::SliderFloat("FOV##2", &cam->fov, 45.f, 179.f);
    ImGui::DragFloat("Yaw##2", &cam->yaw);
    ImGui::DragFloat("Pitch##2", &cam->pitch);
    ImGui::DragFloat3("Position", glm::value_ptr(cam->position));
  }

  // ===== Sun =========================================================================================== //

  assert(sunPtr);
  if (ImGui::CollapsingHeader("Light")) {
    bool updDir = false;

    ImGui::DragFloat("Focus", &sunPtr->focus, 1.f);
    ImGui::SliderFloat("Intensity", &sunPtr->intensity, -10.f, 10.f);
    updDir |= ImGui::SliderAngle("Yaw", &sunPtr->yaw, -180.f, 180.f);
    updDir |= ImGui::SliderAngle("Pitch", &sunPtr->pitch, -90.f, 90.f);
    ImGui::ColorEdit3("Color", glm::value_ptr(sunPtr->color));

    ImGui::SeparatorText("Shadow");
    ImGui::SliderFloat("Size", &sunPtr->shadowSize, 0.f, 1000.f);
    ImGui::SliderFloat("Distance", &sunPtr->shadowDist, 0.f, 1000.f);
    ImGui::SliderFloat("Projection near", &sunPtr->shadowProjNear, 0.f, 10.f);
    ImGui::SliderFloat("Projection far", &sunPtr->shadowProjFar, 0.f, 1000.f);

    if (updDir)
      sunPtr->updateDir();
  };

  // ===== Other ========================================================================================= //

  if (ImGui::CollapsingHeader("Other")) {
  }

  ImGui::End();

  _task.end();

  // ::::: Info window ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImVec2 posBR = viewport->WorkPos + viewport->WorkSize;

  ImGui::SetNextWindowPos(posBR, ImGuiCond_Always, {1.f, 1.f});
  ImGui::SetNextWindowCollapsed(infoCollapsed);

  ImGui::Begin("Info");

  ImGui::Text("FPS: %d / %f.5 ms", fps, global::dt);

  assert(global::profiler);
  global::profiler->renderTasks(400, 200, 200, 0);

  ImGui::End();

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void gui::shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  // ImPlot::DestroyContext();
  ImGui::DestroyContext();
}

