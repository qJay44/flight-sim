#pragma once

#include "texture/Texture2D.hpp"

namespace gfx {

struct Material {
  Texture2D texAlbedo{};
  Texture2D texNormal{};
  Texture2D texORMH{};
  std::string currFolder;

  Material(std::string name, bool flipVertically = false);

  void load(std::string name, bool flipVertically = false);
};

} // namespace gfx

