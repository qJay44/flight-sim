#pragma once

namespace gfx {

class Texture {
public:
  Texture(Texture&& other);
  Texture& operator=(Texture&& other);

  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;

  ~Texture();

  void bind(GLuint unit) const;
  void unbind() const;
  void clear();

  GLuint getId() const;
  GLenum getTarget() const;
  GLenum getInternalFormat() const;

  ivec2 getSize(GLint mipLevel) const;

protected:
  GLenum target = 0;
  GLuint id = 0;

protected:
  Texture() = default;
};

} // namespace gfx

