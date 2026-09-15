#pragma once

namespace gfx {

// Vertex Array Object
struct VAO {
  GLsizei size = 0;
  GLuint id = 0;

  static void unbind() {
    glBindVertexArray(0);
  }

  static const VAO& getEmpty() {
    static VAO emptyVAO(1);
    return emptyVAO;
  }

  VAO() = default;

  VAO(GLsizei size) {
    gen(size);
  }

  VAO(const VAO&) = delete;
  VAO& operator=(const VAO&) = delete;

  VAO(VAO&& other) {
    std::swap(id, other.id);
  }

  VAO& operator=(VAO&& other) {
    if (this != &other)
      std::swap(id, other.id);

    return *this;
  }

  ~VAO() {
    clear();
  }

  void gen(GLsizei size = 1) {
    if (id)
      error("[VAO::gen] VAO is already generated");

    this->size = size;
    glGenVertexArrays(size, &id);
  }

  void bind() const {
    glBindVertexArray(id);
  }

  void clear() {
    if (id) glDeleteVertexArrays(size, &id);
    size = 0;
    id = 0;
  }

  void linkAttrib(GLuint layout, GLuint numComponents, GLenum type, GLsizei stride, void* offset) const {
    glEnableVertexAttribArray(layout);
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
  }
};

} // namespace gfx

