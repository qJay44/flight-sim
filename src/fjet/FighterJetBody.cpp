#include "FighterJetBody.hpp"

#include "PointMass.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/trigonometric.hpp"
#include "utils/types.hpp"
#include "../engine/mesh/fbx/model.hpp"

#include <algorithm>

static vec3 projectOnPlane(vec3 vec, vec3 normal) {
  vec3 n = glm::normalize(normal);
  float d = glm::dot(vec, n);

  return vec - (d * n);
}

float FighterJetBody::getLiftCoeff(float angleRad) {
  float a = glm::degrees(std::abs(angleRad));
  float cl = 0.f;

  if (a < 30.f)
    cl = sin(glm::radians(a * 3.f));
  else if (a < 90.f)
    cl = cos(glm::radians((a - 30.f) * 1.5f));

  return angleRad >= 0.f ? cl : -cl;
}

float FighterJetBody::getLiftCoeffYaw(float angleRad) {
  float a = glm::degrees(std::abs(angleRad));
  if (a > 90.f)
    return 0.f;

  float cl = 1.2f * sin(glm::radians(a * 2.5f));

  return angleRad >= 0.f ? cl : -cl;
}

FighterJetBody::FighterJetBody(const fspath& fbxFilepath, vec3 orientation, float totalMass)
  : localOrientation(orientation)
{
  fbx::Model model = fbx::load(fbxFilepath);
  std::unordered_map<std::string, fbx::UfbxMesh> meshMap;
  std::unordered_map<std::string, mat4> socketMap;

  for (fbx::UfbxMesh& nmesh : model.meshes)
    meshMap[nmesh.name] = std::move(nmesh);

  for (AircraftPart* part : allParts) {
    auto it = meshMap.find(part->name);
    if (it == meshMap.end())
      error("[FighterJetBody::FighterJetBody] Didn't find [{}] in map", part->name);

    part->mesh = std::move(it->second.mesh);
    part->mass = totalMass * part->massPercent;
    part->offset = it->second.averagePos;
  }

  canopy.color = vec3(1.f);

  for (fbx::Socket& s : model.sockets)
    socketMap[s.name] = s.transform;

  afterburner1 = socketMap["Afterburner1"];
  afterburner2 = socketMap["Afterburner2"];
  hardpoint1   = socketMap["Hardpoint1"];
  hardpoint2   = socketMap["Hardpoint2"];

  rigidbody.mass = totalMass;
  rigidbody.position.y = 20.f;
}

const vec3& FighterJetBody::getPosition() const { return rigidbody.position; }
const vec3& FighterJetBody::getVelocity() const { return velocity; }
const glm::quat& FighterJetBody::getOrientation() const { return rigidbody.orientation; }

void FighterJetBody::addThrottle(float input) {
  cfg.throttle = input;
}

void FighterJetBody::update(float dt) {
  calcState(dt);
  calcAngleOfAttack();
  calcGForce(dt);

  updateThrust();
  updateDrag();
  updateLift();
  updateForceFromParts(dt);

  rigidbody.addGravity(-9.81f);
  rigidbody.applyForce(dt);

  updateMesh(dt);
}

void FighterJetBody::draw(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  for (AircraftPart* part : allParts) {
    shader.setUniformMatrix3f("u_localRotation", glm::mat3_cast(part->localRotation));
    part->draw(camera, shader, forceNoWireframe);
  }
}

void FighterJetBody::drawDebug(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  for (AircraftPart* part : allParts)
    part->drawDebug(camera, shader, forceNoWireframe);
}

void FighterJetBody::calcState(float dt) {
  auto invRotation = glm::conjugate(rigidbody.orientation);
  velocity = rigidbody.velocity;
  localVelocity = invRotation * velocity;
  localAngularVelocity = invRotation * rigidbody.angularVelocity;
}

void FighterJetBody::calcAngleOfAttack() {
  if (glm::length2(localVelocity) < 0.1f) {
    angleOfAttack = 0.f;
    angleOfAttackYaw = 0.f;
    return;
  }

  angleOfAttack = atan2(localVelocity.y, -localVelocity.z);
  angleOfAttackYaw = atan2(localVelocity.x, -localVelocity.z);
}

void FighterJetBody::calcGForce(float dt) {
  auto invRotation = glm::conjugate(rigidbody.orientation);
  vec3 acc = (velocity - lastVelocity) / dt;
  localGForce = invRotation * acc;
  lastVelocity = velocity;
}

vec3 FighterJetBody::calcLift(vec3 right, float liftPower, float liftCoeff) {
  vec3 liftVelocity = projectOnPlane(localVelocity, right);
  vec3 liftVelocityNorm = normalizeSafe(liftVelocity);
  float v2 = glm::length2(liftVelocity);

  float liftForce = v2 * liftCoeff * liftPower;
  vec3 liftDir = cross(right, liftVelocityNorm);
  vec3 lift = liftDir * liftForce;

  float dragForce = liftCoeff * liftCoeff * cfg.inducedDrag;
  vec3 dragDir = -liftVelocityNorm;
  vec3 finalInduceDrag = dragDir * v2 * dragForce;

  return lift + finalInduceDrag;
}

void FighterJetBody::updateThrust() {
  rigidbody.addRelativeForce(cfg.throttle * cfg.maxThrust * localOrientation);
}

void FighterJetBody::updateDrag() {
  if (glm::length2(localVelocity) < 0.1f)
    return;

  float ad = airbrakeDeployed * cfg.airbrakeDrag;
  float fd = flapsDeployed * cfg.flapsDrag;
  float totalForwardCd = Cd_forward + ad + fd;

  vec3 lvAbs = abs(localVelocity);
  vec3 dragForceLocal{
    -localVelocity.x * lvAbs.x * Cd_side,
    -localVelocity.y * lvAbs.y * Cd_vertical,
    -localVelocity.z * lvAbs.z * totalForwardCd
  };

  vec3 dragForceWorld = rigidbody.orientation * dragForceLocal;
  rigidbody.addRelativeForce(dragForceWorld);
}

void FighterJetBody::updateLift() {
  if (glm::length2(localVelocity) < 1.f)
    return;

  float currFlapsLiftPower = flapsDeployed * cfg.flapsLiftPower;
  float currFlapsAOABias = flapsDeployed * cfg.flapsAOABias;

  float flapsCoeff = getLiftCoeff(angleOfAttack + glm::radians(currFlapsAOABias));
  vec3 liftForce = calcLift({1.f, 0.f, 0.f}, cfg.liftPower + currFlapsLiftPower, flapsCoeff);

  float rudderCoeff = getLiftCoeffYaw(angleOfAttackYaw);
  vec3 liftForceYaw = calcLift({0.f, 1.f, 0.f}, cfg.rudderPower, rudderCoeff);

  rigidbody.addRelativeForce(liftForce);
  rigidbody.addRelativeForce(liftForceYaw);
}

void FighterJetBody::updateForceFromParts(float dt) {
  for (AircraftPart* part : allParts) {
    vec3 rotatedOffset = rigidbody.orientation * part->offset;
    vec3 worldPos = rigidbody.position;

    if (worldPos.y < cfg.groundHeight) {
      float depth = glm::clamp(cfg.groundHeight - worldPos.y, 0.f, 0.5f);
      float spring = depth * cfg.stiffness;

      vec3 partVel = rigidbody.velocity + cross(rigidbody.angularVelocity, rotatedOffset);
      float damping = -partVel.y * cfg.dampingCoeff;

      vec3 force{0.f, spring + damping, 0.f};
      rigidbody.force += force;
      rigidbody.torque += cross(rotatedOffset, force);
    }
  }
}

void FighterJetBody::updateMesh(float dt) {
  // Flaps deploy animation
  {
    constexpr float maxAngle = glm::radians(-30.f);
    constexpr float rotSpeed = maxAngle * 0.5f;
    static float currentAngle = 0.f;

    float rotDir = flapsDeployed * 2.f - 1.f;
    currentAngle += rotSpeed * rotDir * dt;
    currentAngle = std::clamp(currentAngle, maxAngle, 0.f);

    auto q = glm::angleAxis(currentAngle, vec3(-1.f, 0.f, 0.f));
    leftFlap.localRotation = rightFlap.localRotation = q;
  }

  // Airbrake deploy animation
  {
    constexpr float maxAngle = glm::radians(10.f);
    constexpr float rotSpeed = maxAngle * 0.5f;
    static float currentAngle = 0.f;

    float rotDir = airbrakeDeployed * 2.f - 1.f;
    currentAngle += rotSpeed * rotDir * dt;
    currentAngle = std::clamp(currentAngle, 0.f, maxAngle);

    auto q = glm::angleAxis(currentAngle, vec3(-1.f, 0.f, 0.f));
    airbrake.localRotation = q;
  }

  mat4 bodyTransform = glm::translate(mat4(1.f), rigidbody.position);
  bodyTransform *= glm::mat4_cast(rigidbody.orientation);
  bodyTransform *= glm::scale(mat4(1.f), vec3(cfg.meshScale));

  for (AircraftPart* part : allParts) {
    mat4 partMove = glm::translate(mat4(1.f), part->offset);
    partMove *= glm::mat4_cast(part->localRotation);

    part->model = bodyTransform * partMove;
  }
}

