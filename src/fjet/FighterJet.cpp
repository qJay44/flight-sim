#include "FighterJet.hpp"

#include "glm/common.hpp"
#include "glm/exponential.hpp"
#include "glm/geometric.hpp"
#include "global.hpp"
#include "../terrain/shared.hpp"

FighterJet::FighterJet(const fspath& fbxFilepath, float jetMass, Font* textFont, Shader* textShader)
  : Moveable({}, -PI_2, 0.f),
    body(fbxFilepath, vec3(0.f, 0.f, 1.f), jetMass),
    hud(textFont, textShader),
    camera(vec3{})
{
  camera.setPosition(body.getPosition() + vec3(0.577f) * camDistance);
  camera.setOrientation(vec3(-0.577f));
  camera.setUp(vec3(0.f, 1.f, 0.f));
  camera.setNearPlane(0.1f);
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
  camDistance = glm::clamp(camDistance, camDistanceMin, camDistanceMax);
}

void FighterJet::roll(float dir) {
  body.controlInput.z += dir * global::dt * 5.f;
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
  camera.setFarPlane(terrain::planetRadius * 20.f);

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
  float altidute = glm::length(terrain::planetPos - body.getPosition());
  altidute -= terrain::planetRadius;

  hud.updateSpeed(glm::length(body.velocity));
  hud.updateAltitude(altidute);
}

void FighterJet::updateCamera() {
  double followSpeed = 25.f;
  double lerpFactor = 1.f - glm::exp(-followSpeed * global::dt);

  dvec3 jetForward = body.rigidbody.orientation * body.localOrientation;
  dvec3 jetUp = body.rigidbody.orientation * vec3(0.f, 1.f, 0.f);
  jetForward = glm::normalize(jetForward);
  jetUp = glm::normalize(jetUp);

  // Look slightly above
  dvec3 camOffsetDir = jetUp * 0.34202 + jetForward * -0.93969;
  camOffsetDir = glm::normalize(camOffsetDir);

  dvec3 camPos = camera.getPosition();
  dvec3 jetPos = body.getPosition();
  dvec3 relativePos = camOffsetDir * (double)camDistance;
  dvec3 targetPos = jetPos + relativePos;
  dvec3 nextPos = glm::mix(camPos, targetPos, lerpFactor);

  // Keep camera look ahead when jet rolls (the jet is visually offsetted from the screen center)
  constexpr double lookAheadDist = 2.f;
  dvec3 lookAtEye = jetPos + jetForward * lookAheadDist;
  dvec3 lookAtCenter = lookAtEye - nextPos;

  // WARNING: Apply new camera Up here?
  // What will happen if jet flyes directly towards +y (the jet is located at +x or -x terrain face)?
  camera.setOrientation(glm::normalize(lookAtCenter));
  camera.setPosition(nextPos);
  camera.setPositionRelative(relativePos);
  camera.update();
}

