#pragma once

#include "Mesh.hpp"
#include "MeshData.hpp"

class MeshArrays final : public Mesh {
public:
  using Mesh::Mesh;

  MeshArrays(const MeshData& data);

  void draw(const Camera* camera, Shader& shader) const override;
  void draw(const Camera* camera, Shader& shader, const mat4& model) const override;
};

