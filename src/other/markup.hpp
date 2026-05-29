#pragma once

#include "../engine/Camera.hpp"

namespace markup {
  inline vec3 crossColor{1.f, 0.f, 0.f};

  void init();
  void onResize();

  void drawCross(const Camera* cam, Shader& shader);
}

