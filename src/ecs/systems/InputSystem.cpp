#include "InputSystem.hpp"

#include "utils/utils.hpp"
#include "../../core/EngineContext.hpp"
#include "../systems/CameraSystem.hpp"
#include "../components/VelocityComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/CameraComponent.hpp"
#include "../../gfx/AssetManager.hpp"
#include "../../gui/gui.hpp"

namespace ecs::InputSystem {

using namespace ecs::component;

namespace {

entt::registry* getRegistryFromGLFW(GLFWwindow* window) {
  entt::registry* registry = static_cast<entt::registry*>(glfwGetWindowUserPointer(window));
  if (!registry)
    error("[InputSystem::keyCallback] registry is nullptr");

  return registry;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  entt::registry* registry = getRegistryFromGLFW(window);
  auto& ctx = registry->ctx().get<core::EngineContext>();

  if (action == GLFW_PRESS)   ctx.keyboardKeys[key] = true;
  if (action == GLFW_RELEASE) ctx.keyboardKeys[key] = false;

  // Pre-gui capture
  if (ctx.guiFocused) {
    if (key == GLFW_KEY_R) {
      if (action == GLFW_PRESS) {
        dvec2 winCenter = ctx.getWinCenter();
        ctx.guiFocused = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPos(window, winCenter.x, winCenter.y);
      }
    } else {
      gui::keyCallback(window, key, scancode, action, mods);
    }
    return;
  }

  // If gui is not capturing
  switch (key) {
    case GLFW_KEY_R:
      if (action == GLFW_PRESS) {
        ctx.guiFocused = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      }
      break;
    case GLFW_KEY_E:
      if (action == GLFW_PRESS) gui::toggleConfig();
      break;
    case GLFW_KEY_C:
      if (action == GLFW_PRESS) gui::toggleInfo();
      break;
    case GLFW_KEY_1:
      if (action == GLFW_PRESS) {
        auto meshView = registry->view<MeshComponent>();
        for (auto entity : meshView) {
          auto& meshComponent = registry->get<MeshComponent>(entity);
          meshComponent.mesh->togglePolygonMode();
        }
      }
      break;
    case GLFW_KEY_2:
      if (action == GLFW_PRESS) {
        static gfx::Mesh* globalAxisPtr = registry->ctx().get<gfx::AssetManager>().getMesh("Axis");

        auto meshView = registry->view<MeshComponent>();
        for (auto entity : meshView) {
          auto& meshComponent = registry->get<MeshComponent>(entity);
          if (globalAxisPtr == meshComponent.mesh) {
            meshComponent.disabled = !meshComponent.disabled;
            break;
          }
        }
      }
      break;
  }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  gui::scrollCallback(window, xoffset, yoffset);
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
  entt::registry* registry = getRegistryFromGLFW(window);
  auto& ctx = registry->ctx().get<core::EngineContext>();

  if (!ctx.guiFocused)
    CameraSystem::onMouseMove(*registry, dvec2(xpos, ypos));
  else
    gui::cursorPosCallback(window, xpos, ypos);
}

} // namespace

void init(entt::registry& registry) {
  auto& ctx = registry.ctx().get<core::EngineContext>();

  glfwSetKeyCallback(ctx.window, keyCallback);
  glfwSetScrollCallback(ctx.window, scrollCallback);
  glfwSetCursorPosCallback(ctx.window, cursorPosCallback);

  glfwSetWindowUserPointer(ctx.window, &registry);
}

void update(entt::registry& registry) {
  auto& ctx = registry.ctx().get<core::EngineContext>();
  vec2 winCenter = ctx.getWinCenter();

  int cursorMode = glfwGetInputMode(ctx.window, GLFW_CURSOR);
  if (cursorMode == GLFW_CURSOR_DISABLED)
    glfwSetCursorPos(ctx.window, winCenter.x, winCenter.y);

  if (ctx.keyboardKeys[GLFW_KEY_ESCAPE])
    glfwSetWindowShouldClose(ctx.window, GLFW_TRUE);

  float forwardMask = ctx.keyboardKeys[GLFW_KEY_W];
  float leftMask    = ctx.keyboardKeys[GLFW_KEY_A];
  float backMask    = ctx.keyboardKeys[GLFW_KEY_S];
  float rightMask   = ctx.keyboardKeys[GLFW_KEY_D];
  float upMask      = ctx.keyboardKeys[GLFW_KEY_SPACE];
  float downMask    = ctx.keyboardKeys[GLFW_KEY_LEFT_CONTROL];

  auto velView = registry.view<CameraComponent, VelocityComponent>();
  for (auto entity : velView) {
    const auto& camComponent = registry.get<CameraComponent>(entity);
    auto& velComponent = registry.get<VelocityComponent>(entity);

    const vec3& orientation = camComponent.cam->orientation;
    const vec3& up = camComponent.cam->up;
    const vec3 right = glm::normalize(glm::cross(orientation, up));

    velComponent.velocity +=  orientation * forwardMask;
    velComponent.velocity += -right       * leftMask;
    velComponent.velocity += -orientation * backMask;
    velComponent.velocity +=  right       * rightMask;
    velComponent.velocity +=  up          * upMask;
    velComponent.velocity += -up          * downMask;
  }
}

} // namespace InputSystem

