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

struct LoaderWidget {
  std::string bufLoad;
  std::string bufSave;

  LoaderWidget(const std::string& name) {
    bufLoad.reserve(256);
    bufSave.reserve(256);
    bufLoad = bufSave =  name;
  }

  bool render(auto& cfg) {
    bool u = false;

    ImGui::InputText(".json##0", bufLoad.data(), 256 * sizeof(char)); ImGui::SameLine();
    if (ImGui::Button("Load") && bufLoad[0]) {
      global::json::loadPreset(cfg, std::format("{}.json", bufLoad));
      u = true;
    }

    ImGui::InputText(".json##1", bufSave.data(), 256 * sizeof(char)); ImGui::SameLine();
    if (ImGui::Button("Save") && bufSave[0])
      global::json::savePreset(cfg, std::format("{}.json", bufSave));

    return u;
  }
};

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

[[maybe_unused]]
void ApplySwizzle(const Texture& tex, const std::array<GLint, 4>& swizzle) {
  tex.bind(0);
  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle.data());
  tex.unbind();
}

void RenderTexture(ImTextureRef tex, ImVec2 size = ImVec2(0, 0)) {
  bool useCustomSize = size.x || size.y;

  vec2 winSize = global::getWinSize();
  float winLongestPart = glm::max(winSize.x, winSize.y);
  ImVec2 imgSize = useCustomSize ? size : ImVec2(vec2(winLongestPart * 0.125f));
  ImVec2 imgUV0 = vec2(0.f, 1.f);
  ImVec2 imgUV1 = vec2(1.f, 0.f);

  ImGui::Image(tex, imgSize, imgUV0, imgUV1);
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

  auto _task = global::profiler.startScopedTaskCpu("gui::draw");

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
      auto& cfg = terrain::Quadnode::gm.cfgTerrain;
      bool u = false;

      u |= ImGui::SliderFloat("Planet radius percent (height scale)", &terrainPtr->planetRadiusPercent, 0.f, 1.f);
      u |= ImGui::SliderFloat("Land threshold start", &cfg.landThresholdA, 0.f, 1.f);
      u |= ImGui::SliderFloat("Land threshold end", &cfg.landThresholdB, 0.f, 1.f);
      u |= ImGui::SliderFloat("Continent frequency", &cfg.continentFreq, 0.f, 100.f);
      u |= ImGui::SliderFloat("Start amplitude", &cfg.initAmplitude, 0.f, 1.f);
      u |= ImGui::SliderFloat("Start frequency", &cfg.initFrequency, 0.f, 10.f);
      u |= ImGui::SliderFloat("Amplitude gain", &cfg.gain, 0.f, 100.f);
      u |= ImGui::SliderFloat("Lacunarity", &cfg.lacunarity, 1.f, 10.f);
      u |= ImGui::SliderFloat("Canyon steps", &cfg.canyonSteps, 1.f, 10.f);
      u |= ImGui::SliderFloat("Mountain displacement frequency (1)", &cfg.fbmOffsetFreq1, 1.f, 100.f);
      u |= ImGui::SliderFloat("Mountain displacement frequency (2)", &cfg.fbmOffsetFreq2, 1.f, 100.f);
      u |= ImGui::SliderFloat("Mountain displacement frequency (3)", &cfg.fbmOffsetFreq3, 1.f, 100.f);
      u |= ImGui::SliderFloat("Mountain twist", &cfg.fbmOffsetTwist, 0.f, 1.f);
      u |= ImGui::SliderFloat("F1 Voroni frequency (1)", &cfg.f1VoronoiFreq1, 0.f, 100.f);
      u |= ImGui::SliderFloat("F1 Voroni frequency (2)", &cfg.f1VoronoiFreq2, 0.f, 100.f);
      u |= ImGui::SliderFloat("F1F2 Voroni frequency (1)", &cfg.f1f2VoronoiFreq1, 0.f, 100.f);
      u |= ImGui::SliderFloat("F1F2 Voroni frequency (2)", &cfg.f1f2VoronoiFreq2, 0.f, 100.f);
      u |= ImGui::SliderFloat("Detail start amplitude", &cfg.detailInitAmplitude, 0.f, 100.f);
      u |= ImGui::SliderFloat("Detail start frequency", &cfg.detailInitFrequency, 0.f, 10.f);
      u |= ImGui::SliderFloat("Detail Amplitude gain", &cfg.detailGain, 0.f, 100.f);
      u |= ImGui::SliderFloat("Detail Lacunarity", &cfg.detailLacunarity, 1.f, 10.f);
      u |= ImGui::SliderInt("Octaves", &cfg.octaves, 1, 10);
      u |= ImGui::SliderInt("Detail octaves", &cfg.detailOctaves, 1, 10);

      ImGui::SeparatorText("Load/Save");
      static LoaderWidget lw("heightmap1");
      if (lw.render(terrain::Quadnode::gm.cfgTerrain))
        u = true;

      if (u)
        terrainPtr->reload();
    }
  }

  if (ImGui::CollapsingHeader("Water")) {
    ImGui::SliderFloat("Radius scale", &terrainPtr->waterRadiusScale, 0.f, 1.1f);
    ImGui::SliderFloat("Foam strength", &terrainPtr->foamSharpness, 0.f, 3.f);
    ImGui::SliderFloat("Wave scale", &terrainPtr->waveScale, 0.f, 10.f);
    auto& water = terrainPtr->water;

    ImGui::SeparatorText("Noise settings");
    {
      bool u = false;
      u |= ImGui::DragFloat("Seed XIr", &water.seed1);
      u |= ImGui::DragFloat("Seed XIi", &water.seed2);

      if (u) {
        water.generateNoise();
        water.generateInitialSpectrum();
      }
    }
    ImGui::SeparatorText("Gaussian noise / Butterfly");
    RenderTexture(water.texNoise.getId()); ImGui::SameLine();
    {
      // ugh....
      vec2 winSize = global::getWinSize();
      float winLongestPart = glm::max(winSize.x, winSize.y);
      ImVec2 texSize{(float)water.logSize, std::min((float)water.size, winLongestPart * 0.125f)};
      RenderTexture(water.texButterfly.getId(), texSize);
    }

    ImGui::SeparatorText("General spectrum settings");
    {
      bool u = false;

      u |= ImGui::SliderFloat("G", &water.g, 0.f, 100.f);
      u |= ImGui::SliderFloat("Depth", &water.depth, 0.f, 100.f);
      u |= ImGui::SliderFloat("Lambda", &water.lambda, 0.f, 1.f);
      u |= ImGui::SliderFloat("Length scale", &water.lengthScale, 0.f, 1000.f);

      if (ImGui::TreeNode("Local spectrum settings")) {
        auto& settings = water.local;
        u |= ImGui::SliderFloat("Scale", &settings.scale, 0.f, 1.f);
        u |= ImGui::SliderFloat("Wind speed", &settings.windSpeed, 0.f, 100.f);
        u |= ImGui::SliderAngle("Wind direction", &settings.windDir, 0.f);
        u |= ImGui::SliderFloat("Fetch", &settings.fetch, 0.f, 1e6f);
        u |= ImGui::SliderFloat("Spread blend", &settings.spreadBlend, 0.f, 1.f);
        u |= ImGui::SliderFloat("Swell", &settings.swell, 0.f, 1.f);
        u |= ImGui::SliderFloat("Peak enhancement", &settings.peakEnhancemnt, 0.f, 100.f);
        u |= ImGui::SliderFloat("Short waves fade", &settings.shortWavesFade, 0.f, 2.f);

        ImGui::TreePop();
      }

      if (ImGui::TreeNode("Swell spectrum settings")) {
        auto& settings = water.swell;
        u |= ImGui::SliderFloat("Scale##2", &settings.scale, 0.f, 1.f);
        u |= ImGui::SliderFloat("Wind speed##2", &settings.windSpeed, 0.f, 100.f);
        u |= ImGui::SliderAngle("Wind direction##2", &settings.windDir, 0.f);
        u |= ImGui::SliderFloat("Fetch##2", &settings.fetch, 0.f, 1e6f);
        u |= ImGui::SliderFloat("Spread blend##2", &settings.spreadBlend, 0.f, 1.f);
        u |= ImGui::SliderFloat("Swell##2", &settings.swell, 0.f, 1.f);
        u |= ImGui::SliderFloat("Peak enhancement##2", &settings.peakEnhancemnt, 0.f, 100.f);
        u |= ImGui::SliderFloat("Short waves fade##2", &settings.shortWavesFade, 0.f, 2.f);

        ImGui::TreePop();
      }

      if (u)
        water.generateInitialSpectrum();
    }

    ImGui::SeparatorText("Precomputed Data / Conjugated Spectrum");
    RenderTexture(water.texPrecomputedData.getId()); ImGui::SameLine();
    RenderTexture(water.texInitialSpectrum.getId());

    ImGui::SeparatorText("Displacement / Derivatives / Turbulence");
    RenderTexture(water.texDisplacement.getId()); ImGui::SameLine();
    RenderTexture(water.texDerivatives.getId()); ImGui::SameLine();

    ApplySwizzle(water.texTurbulence, {GL_RED, GL_RED, GL_RED, GL_ONE});
    RenderTexture(water.texTurbulence.getId());

    ImGui::SeparatorText("Load/Save");
    static LoaderWidget lw("tessendorf0");
    if (lw.render(water))
      water.markForRebuild();
  }

  if (ImGui::CollapsingHeader("Postprocess")) {
    // ImGui::SliderFloat("Atmosphere scale", &terrainPtr->atmosphereScale, 0.f, 20.f);
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

  // ::::: Profiler window ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

  ImGui::SetNextWindowCollapsed(infoCollapsed);
  global::profiler.renderTasks(400, 200, 200, 0);

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

