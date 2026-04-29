#pragma once

#include "../engine/mesh/meshes.hpp"
#include "glm/gtc/quaternion.hpp"

struct AircraftPart {
  std::string name;
  float massPercent;

  Mesh mesh;
  float mass;
  vec3 offset;
  vec3 color{0.24377f, 0.355047f, 0.6226415f};
  glm::quat localRotation{1.f, 0.f, 0.f, 0.f};

  bool bDrawDebug = false;
  Mesh debugMassMesh = meshes::circle();

  void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const {
    shader.setUniform3f("u_color", color);
    mesh.draw(camera, shader);
  }

  void drawDebug(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const {
    if (!bDrawDebug)
      return;

    shader.setUniform3f("u_color", 1.f - color);
    debugMassMesh.draw(camera, shader);
  }
};

