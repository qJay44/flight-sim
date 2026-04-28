#include "FighterJet.hpp"

#include "global.hpp"

FighterJet::FighterJet(const fspath& fbxFilepath, float jetMass)
  : Moveable({}, -PI_2, 0.f),
    body(fbxFilepath, jetMass),
    maxThrust(jetMass * 5.f),
    camera(vec3{})
{
  camera.setPosition(body.getPosition() + vec3(0.577f) * camDistance);
  camera.setOrientation(vec3(-0.577f));
  camera.update();
}

bool FighterJet::isActive() const {
 return &camera == Camera::activeCam;
}

// TODO: Using gamepad's stick should pass value from 0.0 to 1.0?
void FighterJet::moveForward() {
  body.applyThrust(1.f * maxThrust);
}

void FighterJet::onMouseMove(dvec2 mousePos) {
  dvec2 winSize = global::getWinSize();
  dvec2 winCenter = winSize * 0.5;
  dvec2 distFromCenter = mousePos - winCenter;

  vec2 delta = glm::radians(dvec2(camera.getSensitivity()) * distFromCenter / winCenter);
  vec3 camOrientation = camera.getOrientation();

  // No vertical rotation if almost looking down or up
  float cosAngle = dot(camera.getUp(), camOrientation);
  if (cosAngle * glm::sign(delta.y) > 0.99f)
    delta.y = 0.f;

  // Horizontal
  glm::quat q = glm::angleAxis(delta.x, camera.getUp());
  camOrientation = q * camOrientation;

  // Vertical
  q = glm::angleAxis(delta.y, camera.getRight());
  camOrientation = q * camOrientation;

  camera.setOrientation(camOrientation);
}

void FighterJet::onMouseScroll(dvec2 offset) {
  camDistance -= offset.y;
  camDistance = glm::clamp(camDistance, 1.f, camDistanceMax);
}

const float& FighterJet::getMaxThrust() const { return maxThrust; }

void FighterJet::setMaxThrust(float t) { maxThrust = t; }

void FighterJet::setCamDistance(float val) {
  camDistance = val;
  camDistanceMax = val * 2.f;
}

void FighterJet::setCamSensitivity(float val) {
  camera.setSensitivity(val);
}

void FighterJet::update() {
  body.update(global::dt);
  updateCamera();
}

void FighterJet::draw(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  body.draw(camera, shader, forceNoWireframe);
}

void FighterJet::drawDebug(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  body.drawDebug(camera, shader, forceNoWireframe);
}

void FighterJet::updateCamera() {
  vec3 actualBack = turnQuat * rotateQuat * camera.getBack();
  vec3 pos = body.getPosition() + actualBack * camDistance;

  camera.setUp(up);
  camera.setOrientation(-actualBack);
  camera.setPosition(pos);
  camera.update();
}

