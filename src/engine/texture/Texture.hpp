#pragma once

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

  const GLuint& getId() const;
  const GLenum& getTarget() const;

  ivec2 getSize(GLint mipLevel) const;

protected:
  GLenum target = 0;
  GLuint id = 0;

protected:
  Texture() = default;
};

