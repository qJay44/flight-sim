#pragma once

#include "Vertex.hpp"
#include "Transformable.hpp"
#include "VAO.hpp"
#include "BufferObject.hpp"
#include "../Camera.hpp"
#include "../Shader.hpp"

class Mesh : public Transformable {
public:
  Mesh() = default;
  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  Mesh(Mesh&& other) = default;
  Mesh& operator=(Mesh&& other) = default;

  Mesh(const std::vector<VertexPCTN>& vertices, const std::vector<GLuint>& indices, GLenum mode, GLenum usage = GL_STATIC_DRAW);
  Mesh(const std::vector<VertexPT>&   vertices, const std::vector<GLuint>& indices, GLenum mode, GLenum usage = GL_STATIC_DRAW);
  Mesh(const std::vector<VertexPC>&   vertices, const std::vector<GLuint>& indices, GLenum mode, GLenum usage = GL_STATIC_DRAW);

  Mesh(const std::vector<VertexPCTN>& vertices, GLenum mode, GLenum usage);
  Mesh(const std::vector<VertexPT>&   vertices, GLenum mode, GLenum usage);
  Mesh(const std::vector<VertexPC>&   vertices, GLenum mode, GLenum usage);

  static Mesh loadObj(const fspath& file, bool printInfo = false);
  static void screenDraw(const Camera* camera, Shader& shader);

  template<typename T>
  void updateData(const std::vector<T>& vertices, GLenum usage) {
    vbo.allocate(vertices, usage);
    count = vertices.size();
  }

  void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;
  void clear();

private:
  GLsizei count = 0;
  GLenum mode = 0;
  VAO vao;
  BufferObject vbo{GL_ARRAY_BUFFER};
  BufferObject ebo{GL_ELEMENT_ARRAY_BUFFER};

  std::function<void(GLenum mode, GLsizei count)> drawFunc;

private:
  static void setCamUniforms(const Camera* c, Shader& s);
  static void setGlobalUniforms(Shader& s);

  static void drawElements(GLenum mode, GLsizei count);
  static void drawArrays(GLenum mode, GLsizei count);

  template<typename V>
  Mesh(const std::span<const V> v, const std::span<const GLuint> i, GLenum mode, GLenum usage)
    : count(i.size() ?: v.size()),
      mode(mode),
      drawFunc(i.size() ? drawElements : drawArrays)
  {
    bool useEBO = i.size();

    vao.gen();
    vbo.allocate(v, usage);
    if (useEBO) ebo.allocate(i, usage);

    vao.bind();
    vbo.bind();
    if (useEBO) ebo.bind();

    V::link(vao);

    vao.unbind();
    vbo.unbind();
    if (useEBO) ebo.unbind();
  }
};


