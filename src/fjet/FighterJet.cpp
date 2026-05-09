#include "FighterJet.hpp"

#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "global.hpp"

FighterJet::FighterJet(const fspath& fbxFilepath, float jetMass)
  : Moveable({}, -PI_2, 0.f),
    body(fbxFilepath, vec3(0.f, 0.f, -1.f), jetMass),
    camera(vec3{})
{
  camera.setPosition(body.getPosition() + vec3(0.577f) * camDistance);
  camera.setOrientation(vec3(-0.577f));
  camera.setFarPlane(1000.f);
  camera.update();
}

// TODO: Using gamepad's stick should pass value from 0.0 to 1.0?
void FighterJet::moveForward() {
  body.cfg.throttle = 1.f;
}

void FighterJet::moveLeft() {
  body.controlInput.y = 1.f;
}

void FighterJet::moveRight() {
  body.controlInput.y = -1.f;
}

void FighterJet::moveUp() {
  body.controlInput.x = 1.f;
}

void FighterJet::moveDown() {
  body.controlInput.x = -1.f;
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

void FighterJet::roll(float dir) {
  body.controlInput.z = dir;
}

bool FighterJet::isActive() const {
 return &camera == Camera::activeCam;
}

FighterJetBody::Config& FighterJet::getBodyConfig() { return body.cfg; }

void FighterJet::setCamDistance(float val) {
  camDistance = val;
  camDistanceMax = val * 2.f;
}

void FighterJet::setCamSensitivity(float val) {
  camera.setSensitivity(val);
}

void FighterJet::toggleAirbrake() {
  body.airbrakeDeployed = !body.airbrakeDeployed;
}
void FighterJet::toggleFlaps() {
  body.flapsDeployed = !body.flapsDeployed;
}

void FighterJet::update() {
  body.update(global::dt);
  updateHUD();
  updateCamera();
}

void FighterJet::draw(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  body.draw(camera, shader, forceNoWireframe);
}

void FighterJet::drawHUD(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  hud.draw(camera, shader);
}

void FighterJet::drawDebug(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  body.drawDebug(camera, shader, forceNoWireframe);
}

void FighterJet::drawDebugBoundaries(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  body.drawDebugBoundaries(camera, shader, forceNoWireframe);
}

void FighterJet::updateHUD() {
  hud.updateSpeed(glm::length(body.velocity));
  hud.updateAltitude(body.getPosition().y);
}

void FighterJet::updateCamera() {
  vec3 actualBack = turnQuat * rotateQuat * camera.getBack();
  vec3 pos = body.getPosition() + actualBack * camDistance;

  camera.setUp(up);
  camera.setOrientation(-actualBack);
  camera.setPosition(pos);
  camera.update();
}

