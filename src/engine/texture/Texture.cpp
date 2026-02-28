#include "Texture.hpp"

Texture::Texture(Texture&& other)
  : desc(other.desc),
    id(other.id)
{
  other.id = 0;
}

Texture& Texture::operator=(Texture&& other) {
  if (this != &other) {
    clear();
    desc = other.desc;
    id = other.id;
    other.id = 0;
  }

  return *this;
}

Texture::~Texture() {
  clear();
}

void Texture::bind(GLuint customUnit) const {
  glActiveTexture(GL_TEXTURE0 + customUnit);
  glBindTexture(desc.target, id);
}

void Texture::bind() const {
  bind(desc.unit);
}

void Texture::unbind() const {
  glBindTexture(desc.target, 0);
}

void Texture::clear() {
  if (id) glDeleteTextures(1, &id);
  id = 0;
}

const GLuint& Texture::getId() const { return id; }
const GLenum& Texture::getTarget() const { return desc.target; }
const GLuint& Texture::getUnit() const { return desc.unit; }
const std::string& Texture::getUniformName() const { return desc.uniformName; }

ivec2 Texture::getSize(GLint mipLevel) const {
  ivec2 res;

  bind();
  glGetTexLevelParameteriv(desc.target, mipLevel, GL_TEXTURE_WIDTH, &res.x);
  glGetTexLevelParameteriv(desc.target, mipLevel, GL_TEXTURE_HEIGHT, &res.y);
  unbind();

  return res;
}

void Texture::setUnit(GLuint unit) {
  desc.unit = unit;
}

void Texture::setUniformName(const std::string& name) {
  desc.uniformName = name;
}

Texture::Texture(const TextureDescriptor& desc) : desc(desc) {}

