#include "FighterJet.hpp"

#include "glm/common.hpp"
#include "glm/exponential.hpp"
#include "glm/geometric.hpp"
#include "global.hpp"

FighterJet::FighterJet(const fspath& fbxFilepath, float jetMass, Font* textFont, Shader* textShader)
  : Moveable({}, -PI_2, 0.f),
    body(fbxFilepath, vec3(0.f, 0.f, 1.f), jetMass),
    hud(textFont, textShader),
    camera(vec3{})
{
  camera.setPosition(body.getPosition() + vec3(0.577f) * camDistance);
  camera.setOrientation(vec3(-0.577f));
  camera.setUp(vec3(0.f, 1.f, 0.f));
  camera.setFarPlane(4000.f);
  camera.update();
}

void FighterJet::moveForward() {
  body.cfg.throttle = 1.f;
}

void FighterJet::moveBack() {
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
  body.controlInput.z += dir * global::dt * 10.f;
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
  body.isActive = isActive();
  updateHUD();
  updateCamera();
}

void FighterJet::draw(const Camera* camera, Shader& shader) const {
  body.draw(FighterJetBody::DRAW_MESH_DEFAULT, camera, shader);
}

void FighterJet::drawHUD(const Camera* camera, Shader& shader) const {
  hud.draw(camera, shader);
}

void FighterJet::drawDebugMass(const Camera* camera, Shader& shader) const {
  body.draw(FighterJetBody::DRAW_MESH_MASSES, camera, shader);
}

void FighterJet::drawDebugHitboxes(const Camera* camera, Shader& shader) const {
  body.draw(FighterJetBody::DRAW_MESH_HITBOXES, camera, shader);
}

void FighterJet::updateHUD() {
  hud.updateSpeed(glm::length(body.velocity));
  hud.updateAltitude(body.getPosition().y);
}

void FighterJet::updateCamera() {
  float followSpeed = 25.f;
  float lerpFactor = 1.f - glm::exp(-followSpeed * global::dt);

  vec3 jetForward = body.rigidbody.orientation * body.localOrientation;
  vec3 jetUp = body.rigidbody.orientation * vec3(0.f, 1.f, 0.f);
  jetForward = glm::normalize(jetForward);
  jetUp = glm::normalize(jetUp);

  // Look slightly above
  vec3 camOffsetDir = jetUp * 0.34202f + jetForward * -0.93969f;
  camOffsetDir = glm::normalize(camOffsetDir);

  vec3 relativePos = camOffsetDir * camDistance;
  vec3 currPos = camera.getPosition();
  vec3 targetPos = body.getPosition() + relativePos;
  vec3 nextPos = glm::mix(currPos, targetPos, lerpFactor);

  // Keep camera look ahead when jet rolls (the jet is visually offsetted from the screen center)
  float lookAheadDist = 50.f;
  vec3 lookAtEye = body.getPosition() + jetForward * lookAheadDist;
  vec3 lookAtCenter = lookAtEye - nextPos;

  camera.setOrientation(glm::normalize(lookAtCenter));
  camera.setPosition(nextPos);
  camera.setPositionRelative(relativePos);
  camera.update();
}

