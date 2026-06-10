#include "MeshElementsInstancing.hpp"

MeshElementsInstancing::MeshElementsInstancing(const MeshData& data) : MeshElements(data) {}

MeshElementsInstancing::MeshElementsInstancing(MeshElements&& other) {
  *this = std::move(other);
}

MeshElementsInstancing& MeshElementsInstancing::operator=(MeshElements&& other) {
  static_cast<MeshElements&>(*this) = std::move(other);
  return *this;
}

void MeshElementsInstancing::setInstanceVBO(const MeshData& data) {
  vao.bind();
  vboInstance.allocate(data.vertices, data.verticesSize, data.usage);

  vboInstance.bind();
  linkAttributes(data.layout);

  for (GLuint i = 0; i < data.layout.count; i++)
    glVertexAttribDivisor(data.layout.attribs->index + i, 1);

  vao.unbind();
  vboInstance.unbind();
  instanceCount = data.verticesSize / data.layout.stride;
}

void MeshElementsInstancing::setInstanceCount(GLsizei c) {
  instanceCount = c;
}

void MeshElementsInstancing::draw(const Camera* camera, Shader& shader) const {
  this->draw(camera, shader, getModel());
}

void MeshElementsInstancing::draw(const Camera* camera, Shader& shader, GLsizei instanceCount) const {
  assert(mode && elementCount && instanceCount);
  vao.bind();

  camera->setUniforms(shader);
  setGlobalUniforms(shader);
  shader.setUniformMatrix4f("u_model", getModel());
  shader.setUniform1i("u_instanceCount", instanceCount);

  shader.use();
  glDrawElementsInstanced(mode, elementCount, GL_UNSIGNED_INT, 0, instanceCount);

  vao.unbind();
}

void MeshElementsInstancing::draw(const Camera* camera, Shader& shader, const mat4& model) const {
  assert(mode && elementCount && instanceCount);
  vao.bind();

  camera->setUniforms(shader);
  setGlobalUniforms(shader);
  shader.setUniformMatrix4f("u_model", model);
  shader.setUniform1i("u_instanceCount", instanceCount);

  shader.use();
  glDrawElementsInstanced(mode, elementCount, GL_UNSIGNED_INT, 0, instanceCount);

  vao.unbind();
}

