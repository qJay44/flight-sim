#pragma once

#include "FighterJetBody.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

class FighterJet : public Moveable {
public:
  FighterJet(const fspath& fbxFilepath, float jetMass);

  void moveForward() override;
  void onMouseMove(dvec2 mousePos) override;
  void onMouseScroll(dvec2 offset) override;

  bool isActive() const;

  FighterJetBody& getBody();

  void setCamDistance(float val);
  void setCamSensitivity(float val);
  void setMeshScale(float s);

  void update();
  void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;
  void drawDebug(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;

private:
  friend struct gui;

  FighterJetBody body;

  Camera camera;
  float camDistance = 10.f;
  float camDistanceMax = 20.f;

  glm::quat turnQuat   = glm::identity<glm::quat>();
  glm::quat rotateQuat = glm::identity<glm::quat>();

private:
  void updateCamera();
};

