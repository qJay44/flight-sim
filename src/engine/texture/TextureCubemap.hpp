#pragma once

#include "Texture.hpp"
#include "image2D.hpp"

class TextureCubemap : public Texture {
public:
  TextureCubemap() = default;
  TextureCubemap(const TextureDescriptor& desc);

  void loadFromImage(const fspath& path);
  void loadFromImage(const image2D& img);
};

