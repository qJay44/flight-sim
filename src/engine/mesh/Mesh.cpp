#include "Mesh.hpp"

#include "vertex.hpp"
#include "global.hpp"
#include "VAO.hpp"

void Mesh::linkAttributes(const MeshData& data) {
  size_t offset = 0;
  for (size_t i = 0; i < data.layout.count; i++) {
    const auto& attr = data.layout.attribs[i];
    glEnableVertexAttribArray(i);
    glVertexAttribPointer(i, attr.size, attr.type, GL_FALSE, data.layout.stride, (void*)offset);
    offset += attr.size * sizeof(data.vertices[0]);
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

void Mesh::updateBufferVBO(const MeshData& data) {
  vbo.updateSubData(data.vertices, data.verticesSize);
  count = data.verticesSize / data.layout.stride;
}

void Mesh::setGlobalUniforms(Shader& s) {
  s.setUniform1f("u_time", global::time);
}

