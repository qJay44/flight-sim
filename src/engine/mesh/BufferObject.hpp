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

  void allocate(const void* data, GLsizeiptr dataSize, GLenum usage) const {
    bind();
    glBufferData(target, dataSize, data, usage);
    unbind();
  }

  void allocate(std::ranges::contiguous_range auto const& container, GLenum usage) const {
    auto s = std::span(container);
    allocate(s.data(), s.size_bytes(), usage);
  }

  void bind() const {
    glBindBuffer(target, id);
  }

  void base(GLuint index) const {
    glBindBufferBase(target, index, id);
  }

  void update(const void* data, GLsizeiptr dataSize, GLbitfield access) const {
    bind();

    void* ptr = glMapBufferRange(target, 0, dataSize, access);
    memcpy(ptr, data, dataSize);

    glUnmapBuffer(target);
    unbind();
  }

  void update(std::ranges::contiguous_range auto const& container, GLbitfield access) const {
    auto s = std::span(container);
    update(s.data(), s.size_bytes(), access);
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

