#pragma once

#include "vertex.hpp"

struct MeshData {
  void* vertices        = nullptr;
  size_t verticesSize   = 0;
  GLuint* indices       = nullptr;
  size_t indicesSize    = 0;
  vertex::Layout layout = {0};
  GLenum usage          = GL_STATIC_DRAW;
  GLenum mode           = GL_TRIANGLES;

  MeshData() = default;

  template<vertex::IsVertexType T>
  MeshData(std::vector<T>& vertices)
    : vertices((float*)vertices.data()),
      verticesSize(vertices.size() * sizeof(T)),
      layout(T::getLayout()) {}

  template<vertex::IsVertexType T>
  MeshData(std::vector<T>& vertices, std::vector<GLuint>& indices)
    : vertices((float*)vertices.data()),
      verticesSize(vertices.size() * sizeof(T)),
      indices(indices.data()),
      indicesSize(indices.size() * sizeof(indices[0])),
      layout(T::getLayout()) {}
};

