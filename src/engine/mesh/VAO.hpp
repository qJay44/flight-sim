#pragma once

// Vertex Array Object
struct VAO {
  static void unbind() {
    glBindVertexArray(0);
  }

  static const VAO& getEmpty() {
    static VAO emptyVAO;

    if (!emptyVAO.id)
      emptyVAO.gen();

    return emptyVAO;
  }

  VAO() = default;

  VAO(const VAO&) = delete;
  VAO& operator=(const VAO&) = delete;

  VAO(VAO&& other) {
    id = other.id;
    other.id = 0;
  }

  VAO& operator=(VAO&& other) {
    if (this != &other) {
      clear();
      id = other.id;
      other.id = 0;
    }

    return *this;
  }


  void gen() {
    glGenVertexArrays(size, &id);
  }

  void bind() const {
    glBindVertexArray(id);
  }

  void clear() {
    if (id) glDeleteVertexArrays(size, &id);
    id = 0;
  }

  void linkAttrib(GLuint layout, GLuint numComponents, GLenum type, GLsizei stride, void* offset) const {
    glEnableVertexAttribArray(layout);
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
  }

private:
  static constexpr GLsizei size = 1;
  GLuint id = 0;
};

