#include "Texture.hpp"
#include <cassert>
#include <utility>

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

const GLuint& Texture::getId() const { return id; }
const GLenum& Texture::getTarget() const { return target; }

ivec2 Texture::getSize(GLint mipLevel) const {
  ivec2 res;

  bind(0);
  glGetTexLevelParameteriv(target, mipLevel, GL_TEXTURE_WIDTH, &res.x);
  glGetTexLevelParameteriv(target, mipLevel, GL_TEXTURE_HEIGHT, &res.y);
  unbind();

  return res;
}

