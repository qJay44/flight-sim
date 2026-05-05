#pragma once

#include "Mesh.hpp"

namespace meshes {

Mesh line(vec3 p1, vec3 p2, vec3 color);
Mesh axis();
Mesh rectangle(GLenum mode = GL_TRIANGLES);
Mesh plane(size_t resolution, GLenum mode = GL_TRIANGLES, vec3 up = {0.f, 1.f, 0.f});
Mesh circle(int resolution = 60);

} // namespace meshes

