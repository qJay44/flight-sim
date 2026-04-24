#pragma once

#include "../engine/mesh/Mesh.hpp"

struct FighterJetMesh : public Moveable {
  Mesh fuselage;
  Mesh nose;
  Mesh cockpit;
  Mesh upperFuselage;
  Mesh engines;
  Mesh wings;
  Mesh leftAileron;
  Mesh rightAileron;
  Mesh leftFlap;
  Mesh rightFlap;
  Mesh leftElevator;
  Mesh rightElevator;
  Mesh rudders;
  Mesh leftRudder;
  Mesh rightRudder;
  Mesh canopy;
  Mesh airbrake;
  mat4 afterburner1; // left
  mat4 afterburner2; // right
  mat4 hardpoint1; // Weapon mount point (under left wing)
  mat4 hardpoint2; // Weapon mount point (under right wing)

  FighterJetMesh(const fspath& fbxFilepath);

  void translateAll(vec3 v);
  void rotateAll(glm::quat q);

  void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;
};

