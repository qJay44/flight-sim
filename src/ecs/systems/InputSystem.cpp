#include "InputSystem.hpp"

#include "utils/utils.hpp"
#include "../../core/EngineContext.hpp"
#include "../../core/ActiveCamera.hpp"
#include "../systems/CameraSystem.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/CameraComponent.hpp"
#include "../components/InputComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/AuxiliaryComponent.hpp"
#include "../components/TerrainComponent.hpp"
#include "../../gfx/AssetManager.hpp"
#include "../../gui/gui.hpp"

namespace ecs::system::InputSystem {

using namespace ecs::component;

namespace {

entt::registry* getRegistryFromGLFW(GLFWwindow* window) {
  entt::registry* registry = static_cast<entt::registry*>(glfwGetWindowUserPointer(window));
  if (!registry)
    error("[InputSystem::getRegistryFromGLFW] registry is nullptr");

  return registry;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  entt::registry* registry = getRegistryFromGLFW(window);
  auto& ctx = registry->ctx().get<core::EngineContext>();
  auto& activeCam = registry->ctx().get<core::ActiveCamera>();

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
    case GLFW_KEY_T:
      if (action == GLFW_PRESS) gui::toggleConfig();
      break;
    case GLFW_KEY_C:
      if (action == GLFW_PRESS) gui::toggleInfo();
      break;
    case GLFW_KEY_F:
      if (action == GLFW_PRESS) {
        for (auto entity : registry->view<CameraComponent>()) {
          auto& camComponent = registry->get<CameraComponent>(entity);

          if (activeCam.cam != camComponent.cam) {
            activeCam.cam = camComponent.cam;
            break;
          }
        }
      }
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
      // Toggling global axis
      if (action == GLFW_PRESS) {
        static gfx::Mesh* globalAxisPtr = registry->ctx().get<gfx::AssetManager>().getMesh("Axis");

        auto meshView = registry->view<MeshComponent, TransformComponent, AuxiliaryComponent>();
        for (auto entity : meshView) {
          auto& meshComponent = registry->get<MeshComponent>(entity);
          if (globalAxisPtr == meshComponent.mesh) {
            meshComponent.disabled = !meshComponent.disabled;
            break;
          }
        }
      }
      break;
    case GLFW_KEY_3:
      if (action == GLFW_PRESS) {
        for (auto entity : registry->view<terrain::TerrainComponent, CameraComponent>()) {
          auto& terrainComponent = registry->get<terrain::TerrainComponent>(entity);
          terrainComponent.renderFrustum = !terrainComponent.renderFrustum;
        }
      }
      break;
    case GLFW_KEY_4:
      if (action == GLFW_PRESS) {
        for (auto entity : registry->view<terrain::TerrainComponent, MeshComponent>()) {
          auto& meshComponent = registry->get<MeshComponent>(entity);
          meshComponent.disabled = !meshComponent.disabled;
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

  if (ctx.guiFocused)
    gui::cursorPosCallback(window, xpos, ypos);
  else
    CameraSystem::onMouseMove(*registry, dvec2(xpos, ypos));
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

  if (ctx.guiFocused)
    return;

  for (auto entity : registry.view<InputComponent>()) {
    auto& inputComponent = registry.get<InputComponent>(entity);

    if (inputComponent.disabled)
      continue;

    inputComponent.keyboardKeys = ctx.keyboardKeys;
  }
}

} // namespace InputSystem

