#pragma once

#include "../../gfx/texture/Texture.hpp"

namespace ecs::component {

struct TextureComponent {
  std::vector<gfx::Texture*> textures; // NOTE: i = unit
};

} // namespace ecs::component

