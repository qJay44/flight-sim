#pragma once

#include "Texture.hpp"
#include "TextureDescriptor.hpp"

namespace gfx {

class Texture2DArray : public Texture {
public:
  using Texture::Texture;

  Texture2DArray() = default;
  Texture2DArray(GLint slots, ivec2 size, const TextureDescriptor& desc);
  Texture2DArray(GLint slots, int size, const TextureDescriptor& desc);

  GLint getSlots(GLint lod) const;

  void upload(GLint slot, ivec2 size, const void* pixels, GLenum format, GLenum type) const;
};

} // namespace gfx

