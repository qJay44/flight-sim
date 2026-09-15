#pragma once

namespace core {

struct Light{
  vec3 color;
  vec3 direction; // Towards light source
  float ambient;
  float specular;
};

} // namespace core

