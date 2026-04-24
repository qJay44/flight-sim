#include "FighterJet.hpp"

#include "global.hpp"

FighterJet::FighterJet(const fspath& fbxFilepath)
  : Moveable({}, -PI_2, 0.f),
    meshes(fbxFilepath),
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
}

void FighterJet::setCamDistance(float val) {
  camDistance = val;
  camDistanceMax = val * 2.f;
}

void FighterJet::update() {
  calcState();
  calcAngleOfAttack();
  calcGForce();

  updateThrust();
  updateCamera();
}

void FighterJet::draw(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  meshes.draw(camera, shader, forceNoWireframe);
}

void FighterJet::calcState() {
  auto invRot = glm::inverse(rotMat);
  localVelocity = invRot * vec4(velocity, 1.f);
  localAngularVelocity = invRot * vec4(angularVelocity, 1.f);
}

void FighterJet::calcAngleOfAttack() {
  if (dot(localVelocity, localVelocity) < 0.1f) {
    angleOfAttack = 0.f;
    angleOfAttackYaw = 0.f;
    return;
  }

  angleOfAttack = atan2(-localVelocity.y, localVelocity.z);
  angleOfAttackYaw = atan2(localVelocity.x, localVelocity.z);
}

void FighterJet::calcGForce() {
  auto invRot = glm::inverse(rotMat);
  auto acceleration = (velocity - lastVelocity) / global::dt;
  localGForce = invRot * vec4(acceleration, 1.f);
  lastVelocity = velocity;
}

void FighterJet::updateThrust() {
  float force = throttle * 5.f;

  throttle *= 0.99f;
  vec3 t = getOrientation() * force;
  meshes.translateAll(t);
}

void FighterJet::updateCamera() {
  vec3 actualBack = turnQuat * rotateQuat * camera.getBack();
  vec3 pos = position + actualBack * camDistance;

  camera.setUp(up);
  camera.setOrientation(-actualBack);
  camera.setPosition(pos);
  camera.update();
}

