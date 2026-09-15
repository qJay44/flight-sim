#include "Mesh.hpp"

#include "draw.hpp"
#include "utils/utils.hpp"
#include "vertex.hpp"
#include <cassert>

namespace gfx {

Mesh::Mesh(const MeshData& data) {
  assert(data.vertices);

  vao.gen();
  vbo.gen();

  vao.bind();
  vbo.allocate(data.vertices, data.verticesSize, data.usage);

  vbo.bind();

  if (data.indices) {
    ebo.gen();
    ebo.allocate(data.indices, data.indicesSize, data.usage);
    ebo.bind();

    if (data.instancing)
      drawCmd = ElementsInstancedDraw{
        .mode = data.mode,
        .indexCount = static_cast<GLsizei>(data.indicesSize / sizeof(data.indices[0])),
        .indexType = GL_UNSIGNED_INT,
        .indicesOffset = nullptr,
        .instanceCount = 1
      };
    else
      drawCmd = ElementsDraw{
        .mode = data.mode,
        .indexCount = static_cast<GLsizei>(data.indicesSize / sizeof(data.indices[0])),
        .indexType = GL_UNSIGNED_INT,
        .indicesOffset = nullptr,
      };
  } else {
    drawCmd = ArraysDraw{
      .mode = data.mode,
      .vertexCount = static_cast<GLsizei>(data.verticesSize / data.layout.stride),
    };
  }

  linkAttributes(data.layout);

  vao.unbind();
  vbo.unbind();
  // No need to unbind ebo here
}

void Mesh::togglePolygonMode() {
  polygonMode = polygonMode == GL_FILL ? GL_LINE : GL_FILL;
}

void Mesh::setInstanceCount(int n) {
  std::get<ElementsInstancedDraw>(drawCmd).instanceCount = n;
}

void Mesh::linkAttributes(const vertex::Layout& layout) {
  size_t offset = 0;

  for (size_t i = 0; i < layout.count; i++) {
    const auto& attr = layout.attribs[i];
    glEnableVertexAttribArray(attr.index);

    size_t elementSize = 0;
    switch (attr.type) {
      case GL_FLOAT:
        glVertexAttribPointer(attr.index, attr.size, attr.type, GL_FALSE, layout.stride, (void*)offset);
        elementSize = sizeof(float);
        break;
      case GL_INT:
        glVertexAttribIPointer(attr.index, attr.size, attr.type, layout.stride, (void*)offset);
        elementSize = sizeof(int);
        break;
      default:
        error("[Mesh::linkAttributes] Unexpected attribute type [{:#x}]", attr.type);
    }

    offset += attr.size * elementSize;
  }
}

} // namespace gfx

