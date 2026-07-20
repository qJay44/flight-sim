#pragma once

#include "ProfilerManager.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "nlohmann/json.hpp"

namespace global {

static const union { vec3 right   {1.f, 0.f, 0.f}, red;   };
static const union { vec3 up      {0.f, 1.f, 0.f}, green; };
static const union { vec3 forward {0.f, 0.f, 1.f}, blue;  };

extern GLFWwindow* window;
extern ProfilerManager profiler;

extern float dt;
extern float time;

extern bool wireframeMode;
extern bool guiFocused;
extern bool drawGrid;
extern bool drawWorldAxis;

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

namespace json {
  void loadPreset(auto& cfg, std::string_view name) {
    fspath path = fspath("res/data/cfg") / name;

    std::ifstream f(path);

    if (f.is_open()) {
      nlohmann::json j;
      f >> j;
      j.get_to(cfg);
      f.close();
    } else {
      warning("[Water::loadPreset] Could not open the file [{}]", path.string());
    }
  }

  void savePreset(auto& cfg, std::string_view name) {
    fspath path = fspath("res/data/cfg") / name;
    std::filesystem::create_directories(path.parent_path());

    std::ofstream f(path);

    if (f.is_open()) {
      nlohmann::json j = cfg;
      f << j.dump(2);
      f.close();
    } else {
      error("[Water::savePreset] Could not open the file [{}]", path.string());
    }
  }

} // namespace json

} // namespace global

