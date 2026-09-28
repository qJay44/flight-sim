#pragma once

#include "Mesh.hpp"
#include "../../core/Camera.hpp"

namespace gfx::frustum {

Mesh create(const core::Camera& cam);
void update(Mesh& mesh, const core::Camera& cam);

} // namespace gfx::frustum

