#include "Texture.hpp"

namespace gfx {

Texture::Texture(Texture&& other)
  : target(other.target),
    id(other.id)
{
  other.id = 0;
}

Texture& Texture::operator=(Texture&& other) {
  if (this != &other) {
    clear();
    std::swap(id, other.id);
    std::swap(target, other.target);
  }

  return *this;
}

Texture::~Texture() {
  clear();
}

void Texture::bind(GLuint unit) const {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(target, id);
}

void Texture::unbind() const {
  glBindTexture(target, 0);
}

void Texture::clear() {
  if (id) glDeleteTextures(1, &id);
  target = 0;
  id = 0;
}

GLuint Texture::getId() const { return id; }
GLenum Texture::getTarget() const { return target; }

GLenum Texture::getInternalFormat() const {
  GLint internalFormat;
  glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

  return internalFormat;
}

ivec2 Texture::getSize(GLint mipLevel) const {
  ivec2 res;

  bind(0);
  glGetTexLevelParameteriv(target, mipLevel, GL_TEXTURE_WIDTH, &res.x);
  glGetTexLevelParameteriv(target, mipLevel, GL_TEXTURE_HEIGHT, &res.y);
  unbind();

  return res;
}

} // namespace gfx

