#include "FighterJet.hpp"

#include "glm/geometric.hpp"
#include "global.hpp"

FighterJet::FighterJet(const fspath& fbxFilepath, float jetMass)
  : Moveable({}, -PI_2, 0.f),
    body(fbxFilepath, vec3(0.f, 0.f, 1.f), jetMass),
    camera(vec3{})
{
  camera.setPosition(body.getPosition() + vec3(0.577f) * camDistance);
  camera.setOrientation(vec3(-0.577f));
  camera.setUp(vec3(0.f, 1.f, 0.f));
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
  body.controlInput.x = -1.f;
}

void FighterJet::moveDown() {
  body.controlInput.x =  1.f;
}

void FighterJet::onMouseMove(dvec2 mousePos) {
  camera.onMouseMove(mousePos);
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
  if (bDrawHUD)
    hud.draw(camera, shader);
}

void FighterJet::drawDebugMass(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  if (bDrawDebugMass)
    body.drawDebugMass(camera, shader, forceNoWireframe);
}

void FighterJet::drawDebugBoundaries(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  if (bDrawDebugBoundaries)
    body.drawDebugBoundaries(camera, shader, forceNoWireframe);
}

void FighterJet::updateHUD() {
  hud.updateSpeed(glm::length(body.velocity));
  hud.updateAltitude(body.getPosition().y);
}

void FighterJet::updateCamera() {
  vec3 back = camera.getBack();
  vec3 pos = body.getPosition() + back * camDistance;

  camera.setOrientation(-back);
  camera.setPosition(pos);
  camera.update();
}

