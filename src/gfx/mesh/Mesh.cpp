#include "Mesh.hpp"

#include "draw.hpp"
#include "utils/utils.hpp"
#include "vertex.hpp"

namespace gfx {

void Mesh::togglePolygonMode() {
  polygonMode = polygonMode == GL_FILL ? GL_LINE : GL_FILL;
}

void Mesh::setInstanceCount(int n) {
  std::get<ElementsInstancedDraw>(drawCmd).instanceCount = n;
};

void Mesh::updateDataBuffer(const MeshData& data) {
  vbo.updateSubData(data.vertices, data.verticesSize);
}

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

    if (data.instanced)
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

Mesh Mesh::createMeshPlane(size_t resolution, bool skirts, bool instanced) {
  size_t res1 = resolution - 1;
  float invRes1 = 1.f / float(res1);

  size_t baseVertexCount = resolution * resolution;
  size_t skirtVertexCount = skirts ? 4 * 2 * res1 : 0;
  std::vector<vertex::P> vertices(baseVertexCount + skirtVertexCount);

  size_t baseIndexCount = res1 * res1 * 6;
  size_t skirtIndexCount = skirts ? res1 * 4 * 6 : 0;
  std::vector<GLuint> indices(baseIndexCount + skirtIndexCount);
  size_t triIndex = 0;

  for (size_t z = 0; z < resolution; z++) {
    float v = z * invRes1;

    for (size_t x = 0; x < resolution; x++) {
      size_t idx = x + z * resolution;
      float u = x * invRes1;

      vertices[idx].position = vec3(u, 0.f, v) * 2.f - 1.f;

      if (x != res1 && z != res1) {
        indices[triIndex + 0] = idx + resolution + 1;  // 0       2 -------- 1
        indices[triIndex + 1] = idx + 1;               // 1       |          |
        indices[triIndex + 2] = idx;                   // 2       |          |
                                                       //    CCW  |          |
        indices[triIndex + 3] = idx;                   // 2       |          |
        indices[triIndex + 4] = idx + resolution;      // 3       |          |
        indices[triIndex + 5] = idx + resolution + 1;  // 0       3 -------- 0

        triIndex += 6;
      }
    }
  }

  if (skirts) {
    size_t skirtVertexIdx = baseVertexCount;

    const auto addSkirt = [&](size_t origIdxA, size_t origIdxB) {
      vertices[skirtVertexIdx] = vertices[origIdxA];
      vertices[skirtVertexIdx + 1] = vertices[origIdxB];

      indices[triIndex + 0] = origIdxB;
      indices[triIndex + 1] = skirtVertexIdx + 1;
      indices[triIndex + 2] = skirtVertexIdx;

      indices[triIndex + 3] = skirtVertexIdx;
      indices[triIndex + 4] = origIdxA;
      indices[triIndex + 5] = origIdxB;

      triIndex += 6;
      skirtVertexIdx += 2;
    };

    for (size_t i = 0; i < res1; i++) {
      addSkirt(i, i + 1);
      addSkirt(res1 * resolution + i + 1, res1 * resolution + i);
      addSkirt((i + 1) * resolution, i * resolution);
      addSkirt(i * resolution + res1, (i + 1) * resolution + res1);
    }
  }

  auto data = MeshData(vertices, indices);
  data.instanced = instanced;

  return Mesh(data);
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

