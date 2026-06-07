#pragma once

#include <span>

namespace colormaps {

// Stolen from: https://github.com/jgreitemann/colormap/blob/master/include/colormap/palettes.hpp ("jet")
constexpr vec3 _jet[9] = {
  {0.0f, 0.0f, 0.5f},
  {0.0f, 0.0f, 1.0f},
  {0.0f, 0.5f, 1.0f},
  {0.0f, 1.0f, 1.0f},
  {0.5f, 1.0f, 0.5f},
  {1.0f, 1.0f, 0.0f},
  {1.0f, 0.5f, 0.0f},
  {1.0f, 0.0f, 0.0f},
  {0.5f, 0.0f, 0.0f},
};

constexpr std::span<const vec3> jet(_jet);

}

