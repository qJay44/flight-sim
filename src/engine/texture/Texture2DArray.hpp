#pragma once

#include "Texture.hpp"

class Texture2DArray : public Texture {
public:
  Texture2DArray() = default;
  Texture2DArray(GLint slots, ivec2 size, const TextureDescriptor& desc);

  const GLint& getSlots() const;
  const ivec2& getSize() const;

  void upload(GLint slot, const void* pixels, GLenum type) const;

private:
  GLint slots{};
  ivec2 size{};
};

