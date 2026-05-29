#pragma once

#include "vertex.hpp"

struct MeshData {
  float* vertices       = nullptr;
  size_t verticesSize   = 0;
  GLuint* indices       = nullptr;
  size_t indicesSize    = 0;
  vertex::Layout layout = {0};
  GLenum usage          = GL_STATIC_DRAW;
  GLenum mode           = GL_TRIANGLES;
};

