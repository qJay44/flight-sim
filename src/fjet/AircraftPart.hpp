#pragma once

#include "../engine/mesh/meshes.hpp"

struct AircraftPart {
  std::string name;
  float massPercent;

  MeshArrays mesh;
  float mass;
  vec3 offset;
  vec3 color{0.24377f, 0.355047f, 0.6226415f};
  glm::quat localRotation{1.f, 0.f, 0.f, 0.f};
  mat4 modelRelative;
  const mat4 dummyMat0{1.f};

  vec3 localBoxMin;
  vec3 localBoxMax;
  mat4 boxModelRelative;

  MeshArrays debugMeshMass = meshes::circle();
  MeshElements debugMeshHitbox = MeshElements::loadFromOBJ("res/obj/Cube.obj");

  const Mesh* meshes[3] {
    &mesh, &debugMeshMass, &debugMeshHitbox
  };

  const mat4* meshesLocalModels[3] {
    &modelRelative, &modelRelative, &boxModelRelative
  };

  vec3 getLocalBoxSize() const {
    return localBoxMax - localBoxMin;
  }

  vec3 getLocalBoxCenter() const {
    return (localBoxMax + localBoxMin) * 0.5f;
  }

  void draw(u8 meshIdx, const mat4& worldTrans, const mat4& view, const Camera* cam, Shader& shader) const {
    const mat4& localModel = *meshesLocalModels[meshIdx];
    mat4 model = worldTrans * localModel;
    mat4 modelView = view * localModel;

    shader.setUniformMatrix4f("u_modelView", modelView);
    shader.setUniform3f("u_color", glm::pow(color, vec3(2.2f)));
    meshes[meshIdx]->draw(cam, shader, model);
  }
};

