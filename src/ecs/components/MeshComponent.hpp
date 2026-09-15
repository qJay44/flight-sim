#pragma once

#include "../../gfx/mesh/Mesh.hpp"
#include "../../gfx/Shader.hpp"

namespace ecs::component {

struct MeshComponent {
  gfx::Mesh* mesh;
  gfx::Shader* shader;
  bool disabled = false;
};

} // namespace ecs::component

