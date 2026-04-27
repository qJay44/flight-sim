#include "meshes.hpp"

#include "global.hpp"

namespace meshes {

Mesh line(vec3 p1, vec3 p2, vec3 color) {
  std::vector<VertexPC> vertices{
    {p1, color},
    {p2, color}
  };

  return Mesh(vertices, GL_LINES, GL_STATIC_DRAW);
}

Mesh axis() {
  std::vector<VertexPC> vertices{
    {{0.f, 0.f, 0.f}, global::red},
    {{1.f, 0.f, 0.f}, global::red},
    {{0.f, 0.f, 0.f}, global::green},
    {{0.f, 1.f, 0.f}, global::green},
    {{0.f, 0.f, 0.f}, global::blue},
    {{0.f, 0.f, 1.f}, global::blue},
  };

  return Mesh(vertices, GL_LINES, GL_STATIC_DRAW);
}

Mesh plane(vec3 color, GLenum mode) {
  std::vector<VertexPCTN> vertices{
    {{-1.f, -1.f, 0.f}, color, {0.f, 0.f}, {1.f, 0.f, 0.f}},
    {{-1.f,  1.f, 0.f}, color, {0.f, 1.f}, {1.f, 0.f, 0.f}},
    {{ 1.f,  1.f, 0.f}, color, {1.f, 1.f}, {1.f, 0.f, 0.f}},
    {{ 1.f, -1.f, 0.f}, color, {1.f, 0.f}, {1.f, 0.f, 0.f}},
  };

  std::vector<GLuint> indices;

  switch (mode) {
    case GL_TRIANGLES: {
      indices = {
        0, 1, 2,
        2, 3, 0
      };
      break;
    }
    case GL_PATCHES: {
      indices = {
        0, 1,
        2, 3,
      };
      break;
    }
    default:
      error("[meshes::plane] Unhandled mode [{}]", mode);
  }

  return Mesh(vertices, indices, mode);
}

Mesh plane(size_t resolution, GLenum mode, vec3 up) {
  std::vector<VertexPT> vertices;
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

      VertexPT& vert = vertices[idx];
      vert.position = up + pX + pY;
      vert.texture = {percentX, percentY};

      if (x != resolution - 1 && y != resolution - 1)
        appendIndicesFunc(idx);
    }
  }

  return Mesh(vertices, indices, mode);
}

Mesh circle(int resolution) {
  float angleStep = (PI * 2.f) / resolution;
  float theta = 0.f;
  std::vector<VertexP> vertices(resolution);

  for (int i = 0; i < resolution; i++) {
    vertices[i].position = {cos(theta), sin(theta), 0.f};
    theta += angleStep;
  }

  return Mesh(vertices, GL_TRIANGLE_FAN, GL_STATIC_DRAW);
}

} // namespace meshes

