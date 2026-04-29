#pragma once

#include "../Mesh.hpp"
#include "glm/gtc/quaternion.hpp"

namespace fbx {

struct Socket {
  std::string name;
  glm::mat4 transform;
};

struct UfbxMesh {
  std::string name;
  Mesh mesh;
  vec3 averagePos;
};

struct Model {
  std::vector<UfbxMesh> meshes;
  std::vector<Socket> sockets;
};

Model load(const fspath& file, bool printInfo = false);

} // namespace fbx

