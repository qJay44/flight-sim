#include "InputsHandler.hpp"

#include "gui/gui.hpp"
#include "global.hpp"

using global::window;

dvec2 InputsHandler::mousePos;

void InputsHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  switch (key) {
    case GLFW_KEY_R:
      if (action == GLFW_PRESS) {
        global::guiFocused = !global::guiFocused;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + 2 * !global::guiFocused);

        // Prevent moving camera towards cursor after disabling it
        dvec2 winCenter = global::getWinCenter();
        if (!global::guiFocused)
          glfwSetCursorPos(window, winCenter.x, winCenter.y);
      }
      break;
    case GLFW_KEY_E:
      if (action == GLFW_PRESS) gui::toggleConfig();
      break;
    case GLFW_KEY_C:
      if (action == GLFW_PRESS) gui::toggleInfo();
      break;
    case GLFW_KEY_1:
      if (action == GLFW_PRESS && !global::guiFocused)
        global::drawWireframe = !global::drawWireframe;
      break;
    case GLFW_KEY_2:
      if (action == GLFW_PRESS && !global::guiFocused)
        global::drawGlobalAxis = !global::drawGlobalAxis;
      break;
    case GLFW_KEY_3:
      if (action == GLFW_PRESS && !global::guiFocused)
        global::drawNormals = !global::drawNormals;
      break;
  }

  if (global::guiFocused)
    gui::keyCallback(window, key, scancode, action, mods);
}

void InputsHandler::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  if (global::guiFocused) {
    gui::scrollCallback(window, xoffset, yoffset);
  }
}

void InputsHandler::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
  mousePos = {xpos, ypos};
  gui::cursorPosCallback(window, xpos, ypos);
}

void InputsHandler::process(Moveable& entity) {
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);

  if (!global::guiFocused) {
    entity.onMouseMove(mousePos);
    entity.accelerate(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) entity.moveForward();
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) entity.moveLeft();
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) entity.moveBack();
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) entity.moveRight();

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) entity.moveUp();
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) entity.moveDown();
  }

  mousePos = global::getWinCenter();
}

