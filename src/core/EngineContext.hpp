#pragma once

namespace core {

struct EngineContext {
  EngineContext() = default;

  EngineContext(const EngineContext&) = delete;
  EngineContext(EngineContext&&) = default;

  EngineContext& operator=(const EngineContext&) = delete;
  EngineContext& operator=(EngineContext&&) = default;

  ~EngineContext() = default;

  GLFWwindow* window = nullptr;
  double dt = 1e-6;
  double time = 0.0;
  bool keyboardKeys[GLFW_KEY_LAST]{};
  bool guiFocused = false;

  inline ivec2 getWinSize() const {
    ivec2 res;
    glfwGetWindowSize(window, &res.x, &res.y);
    return res;
  }

  inline dvec2 getWinCenter() const {
    return dvec2(getWinSize()) * 0.5;
  }

  inline dvec2 getMousePos() const {
    dvec2 res;
    glfwGetCursorPos(window, &res.x, &res.y);
    return res;
  }

  inline float getAspectRatio_WidthOverHeight() const {
    vec2 winSize = getWinSize();
    return winSize.x / winSize.y;
  }

  inline float getAspectRatio_HeihtOverWidth() const {
    vec2 winSize = getWinSize();
    return winSize.y / winSize.x;
  }
};

} // namespace core

