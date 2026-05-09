#include "gui.hpp"

#include "imgui.h"
// #include "implot.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "glm/gtc/type_ptr.hpp"
#include "global.hpp"
#include <cassert>

static bool configCollapsed = true;
static bool infoCollapsed = true;

Camera* gui::camPtr = nullptr;
Sun* gui::sunPtr = nullptr;
FighterJet* gui::fjetPtr = nullptr;

u16 gui::fps = 1;

namespace {

void TextVec3(const char* label, const vec3& v) {
  ImGui::Text("%s: [%.2f, %.2f, %.2f]", label, v.x, v.y, v.z);
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
  ImGui_ImplGlfw_InitForOpenGL(global::window, true);
  ImGui_ImplOpenGL3_Init();
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
  }

  // ===== F15 Body ====================================================================================== //

  if (ImGui::CollapsingHeader("F15 Body")) {
    auto& body = fjetPtr->body;
    ImGui::SliderFloat("Max thrust", &body.cfg.maxThrust, 0.f, 1e6f);
    TextVec3("Local Velocity", body.localVelocity);
    TextVec3("Local Angular Velocity", body.localAngularVelocity);
    ImGui::Text("Angle of attack     [%.2f]", body.angleOfAttack);
    ImGui::Text("Angle of attack yaw [%.2f]", body.angleOfAttackYaw);

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
      bool showAll = ImGui::Button("Show all"); ImGui::SameLine();
      bool hideAll = ImGui::Button("Hide all");

      for (AircraftPart* p : body.parts) {
        p->bDrawDebug |= showAll;
        p->bDrawDebug &= !hideAll;

        if (ImGui::TreeNode(p->name.c_str())) {
          ImGui::Text("Mass: %.2f kg", p->mass);
          ImGui::ColorEdit3("Color", glm::value_ptr(p->color));
          ImGui::Checkbox("Show", &p->bDrawDebug);

          ImGui::TreePop();
        }
      }
      ImGui::TreePop();
    }
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

    if (ImGui::TreeNode("Flags")) {
      ImGui::CheckboxFlags("Right", &camPtr->flags, CameraFlags_DrawRight);
      ImGui::CheckboxFlags("Up", &camPtr->flags, CameraFlags_DrawUp);
      ImGui::CheckboxFlags("Forward", &camPtr->flags, CameraFlags_DrawForward);

      ImGui::TreePop();
    }
  }

  // ===== Sun =========================================================================================== //

  assert(lightPtr);
  if (ImGui::CollapsingHeader("Light")) {
    ImGui::DragFloat("Focus", &sunPtr->focus, 1.f);
    ImGui::DragFloat("Intensity", &sunPtr->intensity, 1.f);
    ImGui::SliderAngle("Yaw", &sunPtr->yaw, -180.f, 180.f);
    ImGui::SliderAngle("Pitch", &sunPtr->pitch, -90.f, 90.f);
    ImGui::ColorEdit3("Color", glm::value_ptr(sunPtr->color));
  };

  // ===== Other ========================================================================================= //

  if (ImGui::CollapsingHeader("Other")) {
    ImGui::Checkbox("Show global axis", &global::drawGlobalAxis);
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

