#pragma once

namespace gfx {

struct ArraysDraw {
  GLenum mode;
  GLsizei vertexCount;
};

struct ElementsDraw {
  GLenum mode;
  GLsizei indexCount;
  GLenum indexType;
  const void* indicesOffset;
};

struct ElementsInstancedDraw {
  GLenum mode;
  GLsizei indexCount;
  GLenum indexType;
  const void* indicesOffset;
  GLsizei instanceCount;
};

} // namespace gfx

