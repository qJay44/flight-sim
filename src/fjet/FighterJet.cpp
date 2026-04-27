#include "FighterJet.hpp"

#include "glm/gtc/quaternion.hpp"
#include "global.hpp"
#include <algorithm>

FighterJet::FighterJet(const fspath& fbxFilepath, float jetMass)
  : Moveable({}, -PI_2, 0.f),
    body(fbxFilepath, jetMass),
    camera(vec3{})
{
  camera.setPosition(position + getBack() * camDistance);
  camera.setView(this);
  camera.update();
}

bool FighterJet::isActive() const {
 return &camera == Camera::activeCam;
}

void FighterJet::moveForward() {
  addTrottle();
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

void FighterJet::addTrottle() {
  throttle += global::dt * 0.1f;
  throttle = std::min(throttle, 1.f);
}

void FighterJet::setCamDistance(float val) {
  camDistance = val;
  camDistanceMax = val * 2.f;
}

void FighterJet::setCamSensitivity(float val) {
  camera.setSensitivity(val);
}

void FighterJet::update() {
  updateThrust();
  body.update(global::dt);
  updateCamera();

  setTranslationMat(body.physicsCore.position);
  setRotationMat(body.physicsCore.orientation);
}

void FighterJet::draw(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  body.draw(camera, shader, forceNoWireframe);
}

void FighterJet::drawDebug(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  body.drawDebug(camera, shader, forceNoWireframe);
}

void FighterJet::updateThrust() {
  float force = throttle * 5.f;
  vec3 t = getOrientation() * force;

  body.physicsCore.position += t;
  position += t;
  throttle *= 0.99f;
}

void FighterJet::updateCamera() {
  vec3 actualBack = turnQuat * rotateQuat * camera.getBack();
  vec3 pos = position + actualBack * camDistance;

  camera.setUp(up);
  camera.setOrientation(-actualBack);
  camera.setPosition(pos);
  camera.update();
}

