#pragma once

#include "../../gfx/mesh/Mesh.hpp"
#include "../../gfx/Shader.hpp"
#include "../../gfx/FBO.hpp"

namespace ecs::component {

struct MeshComponent {
  gfx::Mesh* mesh;
  gfx::Shader* shader;
  gfx::FBO* fbo = nullptr;
  bool disabled = false;
};

} // namespace ecs::component

