#pragma once

#include "FighterJetBody.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "HUD.hpp"

class FighterJet : public Moveable {
public:
  FighterJet(const fspath& fbxFilepath, float jetMass);

  void moveForward() override;
  void moveLeft() override;
  void moveRight() override;
  void moveUp() override;
  void moveDown() override;
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
  void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;
  void drawHUD(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;
  void drawDebug(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;
  void drawDebugBoundaries(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;

private:
  friend struct gui;

  FighterJetBody body;
  Font font{"res/fonts/FiraCodeNerdFontMono-Regular.ttf", 32};
  HUD hud{&font};

  Camera camera;
  float camDistance = 10.f;
  float camDistanceMax = 20.f;

  glm::quat turnQuat   = glm::identity<glm::quat>();
  glm::quat rotateQuat = glm::identity<glm::quat>();

private:
  void updateHUD();
  void updateCamera();
};

