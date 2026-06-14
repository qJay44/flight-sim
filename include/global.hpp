#pragma once

#include "ProfilerManager.hpp"
#include "glm/ext/matrix_clip_space.hpp"

namespace global {

static const union { vec3 right   {1.f, 0.f, 0.f}, red;   };
static const union { vec3 up      {0.f, 1.f, 0.f}, green; };
static const union { vec3 forward {0.f, 0.f, 1.f}, blue;  };

extern GLFWwindow* window;
extern ProfilerManager* profiler;

extern float dt;
extern float time;

extern bool wireframeMode;
extern bool guiFocused;
extern bool drawGrid;

extern bool jetDrawHUD;
extern bool jetDrawDebugMass;
extern bool jetDrawDebugHitboxes;

static inline ivec2 getWinSize() {
  ivec2 res;
  glfwGetWindowSize(global::window, &res.x, &res.y);
  return res;
}

static inline dvec2 getWinCenter() {
  return dvec2(getWinSize()) * 0.5;
}

static inline dvec2 getMousePos() {
  dvec2 res;
  glfwGetCursorPos(global::window, &res.x, &res.y);
  return res;
}

static inline mat4 getScreenProjection() {
  vec2 winSize = getWinSize();
  return glm::ortho(0.f, winSize.x, 0.f, winSize.y);
}

} // namespace global

