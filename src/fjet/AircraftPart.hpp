#pragma once

#include "../engine/mesh/meshes.hpp"
#include "glm/gtc/quaternion.hpp"

struct AircraftPart {
  std::string name;
  float massPercent;

  MeshArrays mesh;
  float mass;
  vec3 offset;
  vec3 color{0.24377f, 0.355047f, 0.6226415f};
  glm::quat localRotation{1.f, 0.f, 0.f, 0.f};
  mat4 model;

  vec3 localBoxMin;
  vec3 localBoxMax;
  vec3 boxColor{0.f, 1.f, 0.f};
  mat4 boxModel;

  MeshArrays debugMeshMass = meshes::circle();

  MeshElements debugMeshBB = MeshElements::loadFromOBJ("res/obj/Cube.obj");
  mat4 rectScale{1.f};

  vec3 getLocalBoxSize() const {
    return localBoxMax - localBoxMin;
  }

  vec3 getLocalBoxCenter() const {
    return (localBoxMax + localBoxMin) * 0.5f;
  }

  void draw(const Camera* camera, Shader& shader) const {
    shader.setUniform3f("u_color", color);
    mesh.draw(camera, shader, model);
  }

  void drawDebugMass(const Camera* camera, Shader& shader) const {
    shader.setUniform3f("u_color", 1.f - color);
    debugMeshMass.draw(camera, shader, model);
  }

  void drawDebugBoundaries(const Camera* camera, Shader& shader) const {
    shader.setUniform3f("u_color", boxColor);
    debugMeshBB.draw(camera, shader, boxModel);
  }
};

