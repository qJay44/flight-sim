#pragma once

#include "Texture.hpp"
#include "TextureDescriptor.hpp"

class Texture2DArray : public Texture {
public:
  using Texture::Texture;

  Texture2DArray() = default;
  Texture2DArray(GLint slots, ivec2 size, const TextureDescriptor& desc);

  const GLint& getSlots() const;
  const ivec2& getSize() const;

  void upload(GLint slot, const void* pixels, GLenum format, GLenum type) const;

private:
  GLint slots{};
  ivec2 size{};
};

