#pragma once

#include "../Light.hpp"
#include "../../fjet/FighterJet.hpp"

struct gui {
  static Camera* camPtr;
  static Light* lightPtr;
  static FighterJet* fjetPtr;
  static u16 fps;

  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
  static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

  static void init();
  static void toggleConfig();
  static void toggleInfo();
  static void draw();
  static void shutdown();
};

