#pragma once

#include "Texture.hpp"

#include "image2D.hpp"
#include "TextureDescriptor.hpp"

class Texture2D : public Texture {
public:
  using Texture::Texture;

  Texture2D() = default;

  static const Texture2D& getDebugTex0();
  [[nodiscard]] static Texture2D storage(const ivec2& size, const TextureDescriptor& desc);

  Texture2D(const image2D& img, const TextureDescriptor& desc);
  Texture2D(const ivec2& size , const TextureDescriptor& desc);
  Texture2D(const fspath& path, const TextureDescriptor& desc);

  void upload(ivec2 coord, ivec2 size, const void* data, GLenum format, GLenum type) const;

private:
  static Texture2D debug0Tex;

  void onInit(const TextureDescriptor& desc);
};

