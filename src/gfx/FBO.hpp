#pragma once

#include "texture/Texture.hpp"

namespace gfx {

struct FBO {
  GLsizei size = 0;
  GLuint id = 0;

  FBO() = default;

  FBO(GLsizei size) {
    gen(size);
  }

  FBO(const FBO&) = delete;
  FBO& operator=(const FBO&) = delete;

  FBO(FBO&& other) {
    std::swap(id, other.id);
    std::swap(size, other.size);
  }

  FBO& operator=(FBO&& other) {
    if (this != &other) {
      std::swap(id, other.id);
      std::swap(size, other.size);
    }

    return *this;
  }

  ~FBO() {
    clear();
  }

  static void unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void gen(GLsizei size = 1) {
    if (id)
      error("[FBO::gen] FBO is already generated");

    this->size = size;
    glGenFramebuffers(size, &id);
  }

  void bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
  }

  void attach2D(GLenum attachment, const Texture& tex) const {
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, tex.getTarget(), tex.getId(), 0);
    unbind();
  }

  void clear() {
    if (id) glDeleteFramebuffers(size, &id);
    id = 0;
    size = 0;
  }
};

} // namespace gfx

