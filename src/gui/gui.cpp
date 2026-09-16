#include "gui.hpp"

#include "imgui.h"
// #include "implot.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ProfilerManager.hpp"
#include "components-ui.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../gfx/AssetManager.hpp"

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

  // ::::: Config window ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

  ImGui::SetNextWindowPos({0, 0}, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowCollapsed(configCollapsed);

  auto _task = profiler.startScopedTaskCpu("gui::draw");

  ImGui::Begin("Config");

  if (ImGui::CollapsingHeader("Global light")) {
    auto* light = assetManager.getLight("GlobalLight");
    ImGui::SliderFloat("Ambient", &light->ambient, 0.f, 1.f);
    ImGui::SliderFloat("Specular", &light->specular, 0.f, 1.f);
    ImGui::ColorEdit3("Color", glm::value_ptr(light->color));
  }

  drawCameraUi(registry);
  drawTerrainUi(registry);

  ImGui::End();

  _task.end();

  // ::::: Info window ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImVec2 posBR = viewport->WorkPos + viewport->WorkSize;

  ImGui::SetNextWindowPos(posBR, ImGuiCond_Always, {1.f, 1.f});
  ImGui::SetNextWindowCollapsed(infoCollapsed);

  profiler.renderTasks(400, 200, 200, 0);

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

