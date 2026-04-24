#pragma once

#include "Moveable.hpp"

struct InputsHandler {
  InputsHandler() = delete;

  static dvec2 mousePos;
  static Moveable* activeEntity;

  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
  static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

  static void process(Moveable& entity);
};

