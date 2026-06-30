#pragma once

class Texture {
public:
  Texture(Texture&& other);
  Texture& operator=(Texture&& other);

  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;

  virtual ~Texture() = 0;

  void bind(GLuint unit = 0) const;
  void unbind() const;
  void clear();

  const GLuint& getId() const;
  const GLenum& getTarget() const;

  bool isGenerated() const;

  ivec2 getSize(GLint mipLevel) const;

protected:
  GLuint id = 0;
  GLenum target = 0;

protected:
  Texture() = default;
};

