#include "meshes.hpp"

#include <cstdio>

#include "MeshData.hpp"

namespace meshes {

MeshArrays line(vec3 p1, vec3 p2) {
  vec3 vertices[] = {p1, p2};

  MeshData data;
  data.vertices = (float*)vertices;
  data.verticesSize = sizeof(vertices);
  data.layout = vertex::P_LAYOUT;
  data.mode = GL_LINES;

  return MeshArrays(data);
}

MeshElements rectangle() {
  vertex::PT vertices[] = {
    {{-1.f, -1.f, 0.f}, {0.f, 0.f}},
    {{-1.f,  1.f, 0.f}, {0.f, 1.f}},
    {{ 1.f,  1.f, 0.f}, {1.f, 1.f}},
    {{ 1.f, -1.f, 0.f}, {1.f, 0.f}},
  };

  GLuint indices[] = {
    3, 2, 1,
    1, 0, 3,
  };

  MeshData data{};
  data.vertices = (float*)vertices;
  data.verticesSize = sizeof(vertices);
  data.indices = indices;
  data.indicesSize = sizeof(indices);
  data.layout = vertex::PT_LAYOUT;

  return MeshElements(data);
}

MeshElements plane(size_t resolution, GLenum mode, vec3 up) {
  std::vector<vertex::PT> vertices;
  std::vector<GLuint> indices;
  size_t triIndex = 0;

  const auto appendIndices_TRIANGLES = [&] (size_t idx) {
    indices[triIndex + 0] = idx + resolution + 1;  // 0       2 -------- 1
    indices[triIndex + 1] = idx + 1;               // 1       |          |
    indices[triIndex + 2] = idx;                   // 2       |          |
    //                                                   CCW  |          |
    indices[triIndex + 3] = idx;                   // 2       |          |
    indices[triIndex + 4] = idx + resolution;      // 3       |          |
    indices[triIndex + 5] = idx + resolution + 1;  // 0       3 -------- 0

    triIndex += 6;
  };

  const auto appendIndices_PATCHES = [&] (size_t idx) {
    indices[triIndex + 0] = idx + resolution + 1; // 0
    indices[triIndex + 1] = idx + 1;              // 1
    indices[triIndex + 2] = idx;                  // 2
    indices[triIndex + 3] = idx + resolution;     // 3

    triIndex += 4;
  };

  size_t indicesPerQuad = 0;
  std::function<void(size_t)> appendIndicesFunc;

  switch (mode) {
    case GL_TRIANGLES:
      indicesPerQuad = 6;
      appendIndicesFunc = appendIndices_TRIANGLES;
      break;
    case GL_PATCHES:
      indicesPerQuad = 4;
      appendIndicesFunc = appendIndices_PATCHES;
      break;
    default:
      error("[meshes::plane] Unexpected mode [{}]", mode);
      break;
  }

  vertices.resize(resolution * resolution);
  indices.resize((resolution - 1) * (resolution - 1) * indicesPerQuad);

  up = -up; // To achieve CCW without messing inside the loop
  vec3 axisA = vec3(up.y, up.z, up.x);
  vec3 axisB = cross(up, axisA);

  for (size_t y = 0; y < resolution; y++) {
    float percentY = y / (resolution - 1.f);
    vec3 pY = (percentY - 0.5f) * 2.f * axisB;

    for (size_t x = 0; x < resolution; x++) {
      size_t idx = x + y * resolution;
      float percentX = x / (resolution - 1.f);
      vec3 pX = (percentX - 0.5f) * 2.f * axisA;

      vertex::PT& vert = vertices[idx];
      vert.position = up + pX + pY;
      vert.texture = {percentX, percentY};

      if (x != resolution - 1 && y != resolution - 1)
        appendIndicesFunc(idx);
    }
  }

  MeshData data;
  data.vertices = (float*)vertices.data();
  data.verticesSize = vertices.size() * sizeof(vertices[0]);
  data.indices = indices.data();
  data.indicesSize = indices.size() * sizeof(indices[0]);
  data.layout = vertex::PT_LAYOUT;
  data.mode = mode;

  return MeshElements(data);
}

MeshArrays circle(int resolution) {
  float angleStep = (PI * 2.f) / resolution;
  float theta = 0.f;
  std::vector<vertex::P> vertices(resolution);

  for (int i = 0; i < resolution; i++) {
    vertices[i].position = {cos(theta), sin(theta), 0.f};
    theta += angleStep;
  }

  MeshData data;
  data.vertices = (float*)vertices.data();
  data.verticesSize = vertices.size() * sizeof(vertices[0]);
  data.layout = vertex::P_LAYOUT;
  data.mode = GL_TRIANGLE_FAN;

  return MeshArrays(data);
}

} // namespace meshes

