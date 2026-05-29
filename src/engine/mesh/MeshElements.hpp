#pragma once

#include "BufferObject.hpp"
#include "Mesh.hpp"
#include "MeshData.hpp"

class MeshElements final : public Mesh {
public:
  using Mesh::Mesh;

  static MeshElements loadFromOBJ(const fspath& file, bool printInfo = false);

  MeshElements(const MeshData& data);

  void draw(const Camera* camera, Shader& shader) const override final;
  void draw(const Camera* camera, Shader& shader, const mat4& model) const override final;

  BufferObject ebo{GL_ELEMENT_ARRAY_BUFFER};
private:
};

