#pragma once

#include "texture/Texture2DArray.hpp"

namespace gfx {

struct Materials {
  Texture2DArray texAlbedo{};
  Texture2DArray texNormal{};
  Texture2DArray texORMH{};
  int nextWriteSlot = -1;

  Materials(ivec2 size, int count);
  Materials(int size, int count);

  void add(const std::string& name);
};

} // namespace gfx

