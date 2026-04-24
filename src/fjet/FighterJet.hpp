#pragma once

#include "FighterJetMesh.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

class FighterJet : public Moveable, public Transformable {
public:
  FighterJet(const fspath& fbxFilepath);

  bool isActive() const;

  void moveForward() override;
  void onMouseMove(dvec2 mousePos) override;
  void onMouseScroll(dvec2 offset) override;

  void addTrottle();
  void setCamDistance(float val);

  void update();
  void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;


private:
  FighterJetMesh meshes;
  vec3 velocity{};
  vec3 lastVelocity{};
  vec3 angularVelocity{};
  vec3 localVelocity{};
  vec3 localAngularVelocity{};
  vec3 localGForce{};
  float angleOfAttack = 0.f;
  float angleOfAttackYaw = 0.f;
  float throttle = 0.f;

  Camera camera;
  float camDistance = 10.f;
  float camDistanceMax = 20.f;

  glm::quat turnQuat   = glm::identity<glm::quat>();
  glm::quat rotateQuat = glm::identity<glm::quat>();

private:
  void calcState();
  void calcAngleOfAttack();
  void calcGForce();

  void updateThrust();
  void updateCamera();
};

