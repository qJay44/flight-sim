#pragma once

#include "MeshArrays.hpp"
#include "MeshElements.hpp"

namespace meshes {

MeshArrays line(vec3 p1, vec3 p2);
MeshElements rectangle();
MeshElements plane(size_t resolution, GLenum mode = GL_TRIANGLES);
MeshArrays circle(int resolution = 60);
MeshArrays axis();

} // namespace meshes

