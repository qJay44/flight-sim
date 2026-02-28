#pragma once

#include "TextureDescriptor.hpp"

class Texture {
public:
  Texture(Texture&& other);
  Texture& operator=(Texture&& other);

  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;

  virtual ~Texture() = 0;

  void bind(GLuint customUnit) const;
  void bind() const;
  void unbind() const;
  void clear();

  const GLuint& getId() const;
  const GLenum& getTarget() const;
  const GLuint& getUnit() const;
  const std::string& getUniformName() const;

  ivec2 getSize(GLint mipLevel) const;

  void setUnit(GLuint unit);
  void setUniformName(const std::string& name);

protected:
  TextureDescriptor desc{};
  GLuint id = 0;

protected:
  Texture() = default;
  Texture(const TextureDescriptor& desc);
};

