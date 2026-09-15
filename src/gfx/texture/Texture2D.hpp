#pragma once

#include "Texture.hpp"
#include "TextureDescriptor.hpp"

#include "image2D.hpp"

namespace gfx {

class Texture2D : public Texture {
public:
  using Texture::Texture;

  void initStorage(const image2D& img, const TextureDescriptor& desc);
  void initStorage(ivec2 size, const TextureDescriptor& desc);
  void initStorage(int size, const TextureDescriptor& desc);

  void initImage(const image2D& img, const TextureDescriptor& desc);

  Texture2D() = default;

  // Uses glTexStorage2D
  Texture2D(const image2D& img, const TextureDescriptor& desc);
  Texture2D(const ivec2& size , const TextureDescriptor& desc);
  Texture2D(int size , const TextureDescriptor& desc);
  Texture2D(const fspath& path, const TextureDescriptor& desc);

  void upload(ivec2 coord, ivec2 size, const void* data, GLenum format, GLenum type) const;

private:
  static Texture2D debugTex0;

  void onInit(const TextureDescriptor& desc);
};

} // namespace gfx

