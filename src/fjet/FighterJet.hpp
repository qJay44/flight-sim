#pragma once

#include "FighterJetBody.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "HUD.hpp"

class FighterJet : public Moveable {
public:
  FighterJet(const fspath& fbxFilepath, float jetMass, Font* textFont, Shader* textShader);

  void moveForward() override;
  void moveBack()    override;
  void moveLeft()    override;
  void moveRight()   override;
  void moveUp()      override;
  void moveDown()    override;
  void onMouseMove(dvec2 mousePos) override;
  void onMouseScroll(dvec2 offset) override;
  void roll(float dir);

  bool isActive() const;

  FighterJetBody::Config& getBodyConfig();

  void setCamDistance(float val);
  void setCamSensitivity(float val);

  void toggleAirbrake();
  void toggleFlaps();

  void update();
  void draw(const Camera* camera, Shader& shader) const;
  void drawHUD(const Camera* camera, Shader& shader) const;
  void drawDebugMass(const Camera* camera, Shader& shader) const;
  void drawDebugBoundaries(const Camera* camera, Shader& shader) const;

private:
  friend struct gui;

  FighterJetBody body;
  HUD hud;

  Camera camera;
  float camDistance = 10.f;
  float camDistanceMax = 20.f;
  glm::quat camQuat = glm::identity<glm::quat>();

  // Better to move to main.cpp
  bool bDrawHUD = true;
  bool bDrawDebugMass = false;
  bool bDrawDebugBoundaries = false;

private:
  void updateHUD();
  void updateCamera();
};

