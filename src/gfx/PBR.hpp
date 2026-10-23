#pragma once

#include "texture/Texture2DArray.hpp"

namespace gfx {

struct Material {
  enum TextureIdx {
    IDX_ALBEDO,
    IDX_AO,
    IDX_EMISSIVITY,
    IDX_HEIGHT,
    IDX_METALLIC,
    IDX_NORMAL,
    IDX_ROUGHNESS,
  };

  Texture2DArray textures{};
  fspath currFolder;

  void load(fspath folderPath, bool flipVertically = true);
};

} // namespace gfx

