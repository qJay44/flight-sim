#pragma once

// VBO - GL_ARRAY_BUFFER
// EBO - GL_ELEMENT_ARRAY_BUFFER
// UBO - GL_UNIFORM_BUFFER
// PBO - GL_PIXEL_PACK_BUFFER
// FBO - X
// RBO - X

#include "utils/utils.hpp"

namespace gfx {

struct BufferObject {
  GLsizei size = 0;
  GLenum target = 0;
  GLuint id = 0;

  static BufferObject createUniformBuffer(bool generateNow = false, GLsizei size = 1) {
    return BufferObject{GL_UNIFORM_BUFFER, generateNow, size};
  }

  BufferObject() = default;

  BufferObject(GLenum target, bool generateNow = false, GLsizei size = 1) : target(target) {
    if (generateNow)
      gen(size);
  }

  BufferObject(const BufferObject&) = delete;
  BufferObject& operator=(const BufferObject&) = delete;

  BufferObject(BufferObject&& other) {
    if (target && target != other.target)
      error("[BufferObject::BufferObject] The new target [{}] is different from current [{}]", other.target, target);

    std::swap(id, other.id);
    target = other.target;
  }

  BufferObject& operator=(BufferObject&& other) {
    if (this != &other) {
      if (target && target != other.target)
        error("[BufferObject::operator=] The new target [{}] is different from current [{}]", other.target, target);

      std::swap(id, other.id);
      target = other.target;
    }

    return *this;
  }

  ~BufferObject() {
    clear();
  }

  void gen(GLsizei size = 1) {
    if (id)
      error("[BufferObject::gen] Buffer is already generated");

    this->size = size;
    glGenBuffers(size, &id);
  }

  void bindAs(GLenum target) const {
    glBindBuffer(target, id);
  }

  void bind() const {
    bindAs(target);
  }

  void bindBaseAs(GLenum target, GLuint index) const {
    glBindBufferBase(target, index, id);
  }

  void bindBase(GLuint index) const {
    bindBaseAs(target, index);
  }

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferStorage.xhtml
  void storage(const void* data, GLsizeiptr dataSize, GLbitfield flags = 0) const {
    bind();
    glBufferStorage(target, dataSize, data, flags);
    unbind();
  }

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferData.xhtml
  void allocate(const void* data, GLsizeiptr dataSize, GLenum usage) const {
    bind();
    glBufferData(target, dataSize, data, usage);
    unbind();
  }

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferSubData.xhtml
  void updateSubData(const void* data, GLsizeiptr dataSize, GLintptr offset = 0) const {
    bind();
    glBufferSubData(target, offset, dataSize, data);
    unbind();
  }

  void unbind() const {
    glBindBuffer(target, 0);
  }

  void clear() {
    if (id) glDeleteBuffers(size, &id);
    size = 0;
    id = 0;
  }
};

} // namespace gfx

