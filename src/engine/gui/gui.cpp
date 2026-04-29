#include "gui.hpp"

#include "imgui.h"
// #include "implot.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "glm/gtc/type_ptr.hpp"
#include "global.hpp"
#include <cassert>

using namespace ImGui;

static bool configCollapsed = true;
static bool infoCollapsed = true;

Camera* gui::camPtr = nullptr;
Light* gui::lightPtr = nullptr;
FighterJet* gui::fjetPtr = nullptr;

u16 gui::fps = 1;

namespace {

void TextVec3(const char* label, const vec3& v) {
  Text("%s: [%.2f, %.2f, %.2f]", label, v.x, v.y, v.z);
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

  SetNextWindowPos({0, 0}, ImGuiCond_FirstUseEver);
  SetNextWindowCollapsed(configCollapsed);

  auto _task = global::profiler->startScopedTask("gui::draw");

  Begin("Config");

  ImGui::Text("FPS: %d / %f.5 ms", fps, global::dt);

  // ===== F15 General =================================================================================== //

  assert(fjetPtr);
  if (CollapsingHeader("F15 General")) {
  }

  // ===== F15 Body ====================================================================================== //

  if (CollapsingHeader("F15 Body")) {
    auto& body = fjetPtr->body;
    SliderFloat("Max thrust", &body.maxThrust, 0.f, 1e6f);

    SeparatorText("Physics core");
    {
      auto& core = body.physicsCore;
      Text("Mass: %.4f kg", core.mass);
      TextVec3("Position", core.position);
      TextVec3("Velocity", core.velocity);
      TextVec3("Local Velocity", core.localVelocity);
      TextVec3("Angular Velocity", core.angularVelocity);
      TextVec3("Local Angular Velocity", core.localAngularVelocity);
    }

    SeparatorText("Center of mass");
    bool showAll = Button("Show all"); SameLine();
    bool hideAll = Button("Hide all");

    SeparatorText("Parts");

    for (AircraftPart* p : body.allParts) {
      p->bDrawDebug |= showAll;
      p->bDrawDebug &= !hideAll;

      if (TreeNode(p->name.c_str())) {
        Text("Mass: %.2f kg", p->mass);
        ColorEdit3("Color", glm::value_ptr(p->color));
        Checkbox("Show", &p->bDrawDebug);

        TreePop();
      }
    }
  }

  // ===== Spectate camera =============================================================================== //

  assert(camPtr);
  if (CollapsingHeader("Spectate camera")) {
    SliderFloat("Near##2", &camPtr->nearPlane, 0.01f, 1.f);
    SliderFloat("Far##2", &camPtr->farPlane,  10.f, 1000.f);
    SliderFloat("Speed##2", &camPtr->speedDefault, 1.f, 50.f);
    SliderFloat("FOV##2", &camPtr->fov, 45.f, 179.f);
    DragFloat("Yaw##2", &camPtr->yaw);
    DragFloat("Pitch##2", &camPtr->pitch);
    DragFloat3("Position", glm::value_ptr(camPtr->position));

    if (TreeNode("Flags")) {
      CheckboxFlags("Right", &camPtr->flags, CameraFlags_DrawRight);
      CheckboxFlags("Up", &camPtr->flags, CameraFlags_DrawUp);
      CheckboxFlags("Forward", &camPtr->flags, CameraFlags_DrawForward);

      TreePop();
    }
  }

  // ===== Light ========================================================================================= //

  assert(lightPtr);
  if (CollapsingHeader("Light")) {
    DragFloat3("Position", glm::value_ptr(lightPtr->position));
    DragFloat("Radius", &lightPtr->radius, 1.f, 0.f);
    ColorEdit3("Color", glm::value_ptr(lightPtr->color));
  };

  // ===== Other ========================================================================================= //

  if (CollapsingHeader("Other")) {
    Checkbox("Show global axis", &global::drawGlobalAxis);
  }

  End();

  _task.end();

  // ::::: Info window ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

  const ImGuiViewport* viewport = GetMainViewport();
  ImVec2 posBR = viewport->WorkPos + viewport->WorkSize;

  SetNextWindowPos(posBR, ImGuiCond_Always, {1.f, 1.f});
  SetNextWindowCollapsed(infoCollapsed);

  Begin("Info");

  ImGui::Text("FPS: %d / %f.5 ms", fps, global::dt);

  assert(global::profiler);
  global::profiler->renderTasks(400, 200, 200, 0);

  End();

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

