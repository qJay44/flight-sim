#include "gui.hpp"

#include "imgui.h"
// #include "implot.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "glm/gtc/type_ptr.hpp"
#include "global.hpp"
#include "utils/utils.hpp"
#include <cassert>

static bool configCollapsed = true;
static bool infoCollapsed = true;

Camera* gui::camPtr = nullptr;
Sun* gui::sunPtr = nullptr;
FighterJet* gui::fjetPtr = nullptr;
terrain::Terrain* gui::terrainPtr = nullptr;

u16 gui::fps = 1;

namespace {

ImFont* fontMain = nullptr;

void TextVec3(const char* label, const vec3& v) {
  ImGui::Text("%s: [%.2f, %.2f, %.2f]", label, v.x, v.y, v.z);
}

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

    ImGui::Text("Coord: [%d, %d]", terrainPtr->lastCoord.x, terrainPtr->lastCoord.y);
    ImGui::SliderFloat("Height scale", &terrainPtr->heightScale, 0.f, 100.f);
    if (ImGui::SliderFloat("Chunk scle", &terrainPtr->chunkSize, 0.f, 1000.f))
      terrainPtr->changeScale(terrainPtr->chunkSize);

    ImGui::Checkbox("Show chunk groups ", &terrainPtr->showChunkGroups);

    ImGui::SeparatorText("Erosion config");
    auto& cfg = terrainPtr->erosionConfig;
    bool u = false;

    u |= ImGui::SliderFloat("Scale", &cfg.erosion_scale, 0.08f, 0.25f);
    CreateMyTooltip("The scale of the erosion effect, affecting it both horizontally and vertically");

    u |= ImGui::SliderFloat("Strength", &cfg.erosion_strength, 0.01f, 0.10f);
    CreateMyTooltip(
      "The strength of the erosion effect, affecting the magnitude of "
      "all octaves, and indirectly affecting the directions of the gullies as a result"
    );

    u |= ImGui::SliderFloat("Gully weight", &cfg.erosion_gully_weight, 0.f, 1.f);
    CreateMyTooltip(
      "The magnitude of the gullies as a weight value from 0 to 1. "
      "A value of 0 can sharpen peaks and valleys but feature virtually no gullies. "
      "A value of 1 produces full gullies but may leave peaks and valleys rounded. "
      "Adjusting erosion gully weight while inversely adjusting erosion scale can be "
      "used to control the sharpness of peaks and valleys while leaving gully "
      "magnitudes largely untouched."
    );

    u |= ImGui::SliderFloat("Detail", &cfg.erosion_detail, 0.7f, 3.f);
    CreateMyTooltip(
      "The overall detail of the erosion. Lower values restrict the effect of higher "
      "frequency gullies to steeper slopes."
    );

    u |= ImGui::SliderFloat("Ridge rounding", &cfg.ridgeRounding, 0.1f, 1.f);
    u |= ImGui::SliderFloat("Crease rounding", &cfg.creaseRounding, 0.0f, 1.f);

    u |= ImGui::SliderFloat("Rounding multiplier 1", &cfg.erosion_rounding.z, 0.f, 5.f);
    CreateMyTooltip(
      "Multiplier applied to the initial height function. "
      "E.g. if the height function has noise of 5 times lower frequency "
      "than the largest gullies, a value of 0.2 can compensate for that."
    );

    u |= ImGui::SliderFloat("Rounding multiplier 2", &cfg.erosion_rounding.w, 0.f, 5.f);
    CreateMyTooltip(
      "Multiplier applied to each subsequent gully octave after the first. "
      "Setting it to the same value as the erosion lacunarity will produce "
      "consistent rounding of all octaves."
    );

    u |= ImGui::SliderFloat4("Onset", glm::value_ptr(cfg.erosion_onset), 0.f, 5.f);
    CreateMyTooltip(
      "Control over how far away from ridges/creases the erosion takes effect. "
      "x: Onset used on the initial height function. "
      "y: Onset used on each gully octave. "
      "z: RidgeMap-specific onset used on the initial height function. "
      "w: RidgeMap-specific onset used on each gully octave."
    );

    u |= ImGui::SliderFloat2("Assumed slope", glm::value_ptr(cfg.erosion_assumed_slope), 0.f, 1.f);
    CreateMyTooltip(
      "Control over the assumed slope of the initial height function. "
      "In practise, assuming a slope can work better than using the input slope, "
      "since the final terrain can be shaped quite differently than the input. "
      " x: An assumed slope value to override the actual slope. "
      " y: The amount (from 0 to 1) to override the actual slope."
    );

    u |= ImGui::SliderFloat("Cell scale", &cfg.erosion_cell_scale, 0.f, 1.f);
    CreateMyTooltip(
      "Gullies are based on stripes within Voronoi-like cells in the Phacelle noise "
      "function. The cell scale parameter controls the sizes of the cells relative "
      "to the overall erosion scale, while keeping the stripe widths unaffected. "
      "Values close to 1 usually produce good results. Smaller values produce more "
      "grainy gullies while larger values produce longer unbroken gullies, but too "
      "large values produce chaotic curved gullies that are not aligned with the "
      "slopes. Value changes can cause abrupt changes in output, especially far away "
      "from the origin, so this parameter is not well suited for animation or for "
      "modulation by other functions."
    );

    u |= ImGui::SliderFloat("Normalization", &cfg.erosion_normalization, 0.f, 1.f);
    CreateMyTooltip(
      "The degree of normalization applied in the Phacelle noise, between 0 and 1. "
      "The erosion filter depends on a certain consistency in magnitude of the "
      "Phacelle output. However, high values can create loopy results where ridges "
      "and creases meet up at a point, which produces unnatural looking results."
    );

    u |= ImGui::SliderInt("Octaves", &cfg.erosion_octaves, 0, 20);
    CreateMyTooltip(
      "Control over the erosion octaves, with each successive octave layering "
      "smaller gullies onto the terrain."
    );

    u |= ImGui::SliderFloat("Lacunarity", &cfg.erosion_lacunarity, 0.f, 10.f);
    CreateMyTooltip(
      "The lacunarity controls the frequency (the inverse "
      "horizontal scale) of each octave relative to the last."
    );

    u |= ImGui::SliderFloat("Gain", &cfg.erosion_gain, 0.f, 5.f);
    CreateMyTooltip(
      "The gain controls the magnitude (the vertical "
      "scale) of each octave relative to the last."
    );

    ImGui::SeparatorText("Terrain parameters not used in the erosion function itself.");

    u |= ImGui::SliderFloat2("Terrain height offset", glm::value_ptr(cfg.terrain_height_offset), -1.f, 1.f);
    CreateMyTooltip(
      "Control over whether the erosion effect raises or lowers the terrain. "
      " x: An offset value between -1 and 1, where a value of -1 only lowers, while "
      "    1 only raises. The offset is proportional to the erosion strength "
      "    parameter, so if that parameter is the same for the entire terrain, the "
      "    effect of the height offset will move the entire terrain surface up or "
      "    down by the same emount. "
      " y: A value between 0 and 1 which is the degree to which the offset value is "
      "    replaced by the negated erosion fade target value. This has the effect "
      "    of only raising at valleys and only lowering at peaks, which, due to how "
      "    the erosion filter works, has the effect of largely preserving the minima "
      "    and maxima of the terrain."
    );

    u |= ImGui::SliderFloat("Height frequency", &cfg.height_frequency, 0.f, 10.f);
    CreateMyTooltip("The inverse horizontal scale of the terrain noise function.");

    u |= ImGui::SliderFloat("Height amplitude", &cfg.height_amp, 0.f, 1.f);
    CreateMyTooltip("The vertical scale (amplitude) of the terrain noise function.");

    u |= ImGui::SliderInt("Height octaves", &cfg.height_octaves, 0, 10);
    CreateMyTooltip(
      "Control over the noise function octaves, with each successive "
      "octave layering smaller bumps onto the terrain."
    );

    u |= ImGui::SliderFloat("Height lacunarity", &cfg.height_lacunarity, 0.f, 10.f);
    CreateMyTooltip(
      "The lacunarity controls the frequency (the inverse "
      "horizontal scale) of each octave relative to the last."
    );

    u |= ImGui::SliderFloat("Height gain", &cfg.height_gain, 0.f, 1.f);
    CreateMyTooltip(
      "The gain controls the magnitude (the vertical scale) "
      "of each octave relative to the last."
    );

    u |= ImGui::SliderFloat("Height function scale", &cfg.heightFunctionScale, 0.f, 1.f);

    if (u)
      terrainPtr->regenerateAllChunks();
  }

  // ===== Spectate camera =============================================================================== //

  assert(camPtr);
  if (ImGui::CollapsingHeader("Spectate camera")) {
    ImGui::SliderFloat("Near##2", &camPtr->nearPlane, 0.01f, 1.f);
    ImGui::SliderFloat("Far##2", &camPtr->farPlane,  10.f, 1000.f);
    ImGui::SliderFloat("Speed##2", &camPtr->speedDefault, 1.f, 50.f);
    ImGui::SliderFloat("FOV##2", &camPtr->fov, 45.f, 179.f);
    ImGui::DragFloat("Yaw##2", &camPtr->yaw);
    ImGui::DragFloat("Pitch##2", &camPtr->pitch);
    ImGui::DragFloat3("Position", glm::value_ptr(camPtr->position));
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

