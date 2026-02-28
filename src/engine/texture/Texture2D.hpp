#pragma once

#include "Texture.hpp"

#include "image2D.hpp"

class Texture2D : public Texture {
public:
  static const Texture2D& getDebug0Tex();

  Texture2D() = default;

  Texture2D(const image2D& img, const TextureDescriptor& desc);
  Texture2D(const ivec2& size , const TextureDescriptor& desc);
  Texture2D(const fspath& path, const TextureDescriptor& desc);

  void upload(ivec2 coord, ivec2 size, const void* data, GLenum type) const;

private:
  static Texture2D debug0Tex;
};

