#include "MeshArrays.hpp"

MeshArrays::MeshArrays(const MeshData& data)  {
  mode = data.mode;
  elementCount = data.verticesSize / data.layout.stride;

  vao.gen();

  vao.bind();
  vbo.allocate(data.vertices, data.verticesSize, data.usage);

  vbo.bind();
  linkAttributes(data.layout);

  vao.unbind();
  vbo.unbind();
}

void MeshArrays::draw(const Camera* camera, Shader& shader) const {
  draw(camera, shader, getModel());
}

void MeshArrays::draw(const Camera* camera, Shader& shader, const mat4& model) const {
  vao.bind();

  camera->setUniforms(shader);
  setGlobalUniforms(shader);
  shader.setUniformMatrix4f("u_model", model);

  shader.use();
  glDrawArrays(mode, 0, elementCount);

  vao.unbind();
}

