#include "Mesh.hpp"

#include "utils/utils.hpp"
#include "vertex.hpp"
#include "global.hpp"
#include "VAO.hpp"

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

void Mesh::drawScreen(const Camera* camera, Shader& shader) {
  static const VAO& vao = VAO::getEmpty();
  vao.bind();

  camera->setUniforms(shader);
  setGlobalUniforms(shader);
  shader.use();

  glDrawArrays(GL_TRIANGLES, 0, 6);
  vao.unbind();
}

GLsizei Mesh::getElementCount() const {
  return elementCount;
}

void Mesh::updateBufferVBO(const MeshData& data) {
  vbo.updateSubData(data.vertices, data.verticesSize);
  elementCount = data.verticesSize / data.layout.stride;
}

void Mesh::setGlobalUniforms(Shader& s) {
  s.setUniform1f("u_time", global::time);
  s.setUniform2f("u_resolution", global::getWinSize());
}

