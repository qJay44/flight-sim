#pragma once

#include "vertex.hpp"
#include <cassert>

namespace gfx {

struct MeshData {
  void* vertices        = nullptr;
  size_t verticesSize   = 0;
  GLuint* indices       = nullptr;
  size_t indicesSize    = 0;
  vertex::Layout layout = {0};
  GLenum usage          = GL_STATIC_DRAW;
  GLenum mode           = GL_TRIANGLES;
  bool instanced       = false;

  MeshData() = default;

  template<vertex::IsVertexType T>
  MeshData(std::vector<T>& vertices)
    : vertices(vertices.data()),
      verticesSize(vertices.size() * sizeof(T)),
      layout(T::getLayout())
  {
    assert(this->vertices);
  }

  template<vertex::IsVertexType T>
  MeshData(std::vector<T>& vertices, std::vector<GLuint>& indices)
    : vertices(vertices.data()),
      verticesSize(vertices.size() * sizeof(T)),
      indices(indices.data()),
      indicesSize(indices.size() * sizeof(GLuint)),
      layout(T::getLayout())
  {
    assert(this->vertices);
  }
};

} // namespace gfx

