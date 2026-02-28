#include "gui.hpp"

#include "imgui.h"
// #include "implot.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "glm/gtc/type_ptr.hpp"
#include "global.hpp"

using namespace ImGui;

static bool configCollapsed = true;
static bool infoCollapsed = true;

Camera* gui::camPtr = nullptr;
Light* gui::lightPtr = nullptr;

u16 gui::fps = 1;

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

  // ===== Spectate camera =============================================================================== //

  if (!camPtr) error("The spectate camera is not linked to gui");
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

  if (!lightPtr) error("The light is not linked to gui");
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

