#include "Texture2DArray.hpp"

#include "utils/utils.hpp"

Texture2DArray::Texture2DArray(GLint slots, ivec2 size, const TextureDescriptor& desc)
  : Texture(desc),
    slots(slots),
    size(size)
{
  if (desc.target != GL_TEXTURE_2D_ARRAY)
    error("[Texture2DArray::Texture2DArray] Wrong target for [{}]", desc.uniformName);

  glGenTextures(1, &id);
  bind();
  glTexParameteri(desc.target, GL_TEXTURE_MIN_FILTER, desc.minFilter);
  glTexParameteri(desc.target, GL_TEXTURE_MAG_FILTER, desc.magFilter);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_S, desc.wrapS);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_T, desc.wrapT);
  glTexStorage3D(desc.target, 1, desc.internalFormat, size.x, size.y, slots);
  unbind();
}

const GLint& Texture2DArray::getSlots() const {
  return slots;
}

const ivec2& Texture2DArray::getSize() const {
  return size;
}

void Texture2DArray::upload(GLint slot, const void* pixels, GLenum type) const {
  bind();
  glTexSubImage3D(desc.target, 0, 0, 0, slot, size.x, size.y, 1, desc.format, type, pixels);

  if (desc.genMipMap)
    glGenerateMipmap(desc.target);

  unbind();
}

