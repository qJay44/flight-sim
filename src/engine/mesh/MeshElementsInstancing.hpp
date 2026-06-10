#pragma once

#include "MeshElements.hpp"

class MeshElementsInstancing : public MeshElements {
public:
  using MeshElements::MeshElements;

  MeshElementsInstancing(MeshElements&& other);
  MeshElementsInstancing& operator=(MeshElements&& other);

  MeshElementsInstancing(const MeshData& data);

  void setInstanceVBO(const MeshData& data);
  void setInstanceCount(GLsizei c);

  void draw(const Camera* camera, Shader& shader) const override;
  void draw(const Camera* camera, Shader& shader, GLsizei count) const;
  void draw(const Camera* camera, Shader& shader, const mat4& model) const override;

private:
  GLsizei instanceCount = 0;
  BufferObject vboInstance{GL_ARRAY_BUFFER};
};

