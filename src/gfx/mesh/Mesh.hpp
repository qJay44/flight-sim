#pragma once

#include "../BufferObject.hpp"
#include "VAO.hpp"
#include "draw.hpp"
#include "MeshData.hpp"

namespace gfx {

class Mesh {
public:
  Mesh() = default;
  Mesh(const Mesh&) = delete;
  Mesh(Mesh&&) = default;

  Mesh& operator=(const Mesh&) = delete;
  Mesh& operator=(Mesh&&) = default;
  ~Mesh() = default;

  void togglePolygonMode();
  void convertToInstancing();
  void setInstanceCount(int n);

private:
  friend class Renderer;
  friend class AssetManager;

  VAO vao{};
  BufferObject vbo{GL_ARRAY_BUFFER};
  BufferObject ebo{GL_ELEMENT_ARRAY_BUFFER};
  GLenum polygonMode = GL_FILL;

  std::variant<ArraysDraw, ElementsDraw, ElementsInstancedDraw> drawCmd;

private:
  [[nodiscard]] Mesh(const MeshData& data);

  void linkAttributes(const vertex::Layout& layout);
};

} // namespace gfx

