#pragma once

#include "../Mesh.hpp"

namespace fbx {

struct Socket {
  std::string name;
  glm::mat4 transform;
};

struct NamedMesh {
  std::string name;
  Mesh mesh;
  vec3 averagePos;
};

struct Model {
  std::vector<NamedMesh> meshes;
  std::vector<Socket> sockets;
};

Model load(const fspath& file, bool printInfo = false);

} // namespace fbx

