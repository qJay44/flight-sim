#pragma once

// VBO - GL_ARRAY_BUFFER
// EBO - GL_ELEMENT_ARRAY_BUFFER
// UBO - GL_UNIFORM_BUFFER
// PBO - GL_PIXEL_PACK_BUFFER
// FBO - X
// RBO - X

struct BufferObject {
public:
  BufferObject() = default;

  BufferObject(GLenum target) : target(target) {
    gen();
  }

  BufferObject(const BufferObject&) = delete;
  BufferObject& operator=(const BufferObject&) = delete;

  BufferObject(BufferObject&& other) {
    std::swap(id, other.id);
    target = other.target;
  }

  BufferObject& operator=(BufferObject&& other) {
    if (this != &other) {
      if (target && target != other.target)
        error("[BufferObject] The new target [{}] is different from current [{}]", other.target, target);

      std::swap(id, other.id);
      target = other.target;
    }

    return *this;
  }

  ~BufferObject() {
    clear();
  }

  void gen() {
    glGenBuffers(size, &id);
  }

  void bind() const {
    glBindBuffer(target, id);
  }

  void bindBase(GLuint index) const {
    glBindBufferBase(target, index, id);
  }

  void allocate(const void* data, GLsizeiptr dataSize, GLenum usage) const {
    bind();
    glBufferData(target, dataSize, data, usage);
    unbind();
  }

  void storage(const void* data, GLsizeiptr dataSize, GLbitfield flags) const {
    bind();
    glBufferStorage(target, dataSize, data, flags);
    unbind();
  }

  void* mapRange(GLsizeiptr dataSize, GLbitfield access, GLintptr offset = 0) const {
    bind();
    void* ptr = glMapBufferRange(target, offset, dataSize, access);

    return ptr;
  }

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
    id = 0;
  }

private:
  static constexpr GLsizei size = 1;

  GLenum target = 0;
  GLuint id = 0;
};

