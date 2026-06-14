#include "FighterJetBody.hpp"

#include "../engine/mesh/fbx/model.hpp"
#include "Animation.hpp"
#include "FighterJetBody.hpp"
#include "PointMass.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/trigonometric.hpp"
#include "global.hpp"
#include "utils/types.hpp"

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

  for (AircraftPart* part : parts) {
    auto it = meshMap.find(part->name);
    if (it == meshMap.end())
      error("[FighterJetBody::FighterJetBody] Didn't find [{}] in map", part->name);

    auto& ufbxMesh = it->second;

    part->mesh = std::move(ufbxMesh.mesh);
    part->mass = totalMass * part->massPercent;
    part->offset = ufbxMesh.offset;
    part->localBoxMin = ufbxMesh.minPos;
    part->localBoxMax = ufbxMesh.maxPos;
  }

  canopy.color = vec3(1.f);

  for (fbx::Socket& s : model.sockets)
    socketMap[s.name] = s.transform;

  afterburner1 = socketMap["Afterburner1"];
  afterburner2 = socketMap["Afterburner2"];
  hardpoint1   = socketMap["Hardpoint1"];
  hardpoint2   = socketMap["Hardpoint2"];

  rigidbody.mass = totalMass;
  rigidbody.inverseMass = totalMass == 0.f ? 0.f : 1.f / totalMass;
  rigidbody.position.y = 100.f;
  rigidbody.localInertia = { 471906.f, 684784.f, 212878.f }; // Calculate at runtime?
  rigidbody.drag = 0.02f;
  rigidbody.angularDrag = 0.5f;

  initialRotation = glm::angleAxis(PI, vec3{0.f, 1.f, 0.f});

  float flapsMaxAngle = glm::radians(-20.f);
  animFlaps.maxAngleRad = 0.f;
  animFlaps.minAngleRad = flapsMaxAngle;
  animFlaps.rotSpeedRad = flapsMaxAngle * 2.f;
  animFlaps.rotAxis     = {-1.f, 0.f, 0.f};

  float airbrakeMaxAngle = glm::radians(20.f);
  animAirbrake.maxAngleRad = airbrakeMaxAngle;
  animAirbrake.minAngleRad = 0.f;
  animAirbrake.rotSpeedRad = airbrakeMaxAngle * 2.f;
  animAirbrake.rotAxis     = {-1.f, 0.f, 0.f};

  float pitchMaxAngle = glm::radians(20.f);
  animPitch.maxAngleRad = pitchMaxAngle;
  animPitch.minAngleRad = -pitchMaxAngle;
  animPitch.rotSpeedRad = pitchMaxAngle * 2.f;
  animPitch.rotAxis     = {-1.f, 0.f, 0.f};

  float rollMaxAngle = glm::radians(20.f);
  animRoll.maxAngleRad = rollMaxAngle;
  animRoll.minAngleRad = -rollMaxAngle;
  animRoll.rotSpeedRad = rollMaxAngle * 2.f;
  animRoll.rotAxis     = {0.906308f, 0.0f, -0.422618f}; // The rotated Left by 25 degrees around the Up

  float yawMaxAngle = glm::radians(20.f);
  animYaw.maxAngleRad = yawMaxAngle;
  animYaw.minAngleRad = -yawMaxAngle;
  animYaw.rotSpeedRad = yawMaxAngle * 2.f;
  animYaw.rotAxis     = {0.f, 1.f, 0.f};
}

const vec3& FighterJetBody::getPosition() const { return rigidbody.position; }
const vec3& FighterJetBody::getVelocity() const { return velocity; }
const glm::quat& FighterJetBody::getOrientation() const { return rigidbody.orientation; }

float FighterJetBody::getSpeed() const { return glm::max(0.f, localVelocity.z); }

void FighterJetBody::update(float dt) {
  global::profiler->startScopedTask("FighterJetBodyUpdatePass");

  calcState(dt);
  calcAngleOfAttack();

  updateThrust();
  updateDrag();
  updateLift();
  updateSteering(dt);
  updateForceFromParts(dt);

  rigidbody.addForce({0.f, -9.81f * rigidbody.mass * 2.f, 0.f});
  rigidbody.update(dt, cfg.meshScale * 100.f); // idk

  updateMesh(dt);

  controlInput *= 0.9f;
}

void FighterJetBody::draw(DrawMesh type, const Camera* camera, Shader& shader) const {
  mat4 worldTranslation = glm::translate(mat4(1.f), rigidbody.position);
  mat4 localView;
  if (isActive)
    localView = camera->getLocalView(camera->getPositionRelative());
  else
    localView = camera->getLocalView(camera->getPosition() - rigidbody.position);

  for (const AircraftPart* part : parts)
    part->draw(type, worldTranslation, localView, camera, shader);
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

  angleOfAttack = atan2(-localVelocity.y, localVelocity.z);
  angleOfAttackYaw = atan2(localVelocity.x, localVelocity.z);
}

vec3 FighterJetBody::calcGForceLimit(vec3 controlInput) const {
  vec3 input = controlInput * cfg.gLimit;

  if (input.x < 0.f)
    input.x = controlInput.x * cfg.gLimitPitch;

  return input * 9.81f;
}

vec3 FighterJetBody::calcGForce(vec3 angularVelocity, vec3 vel) const {
  return glm::cross(angularVelocity, vel);
}

float FighterJetBody::calcGLimitter(vec3 controlInput, vec3 maxAngularVelocity) {
  vec3 maxInput = normalizeSafe(controlInput);
  vec3 limit = calcGForceLimit(maxInput);
  vec3 maxGForce = calcGForce(maxInput * maxAngularVelocity, localVelocity);

  float maxGForceLen = glm::length(maxGForce);
  float limitLen = glm::length(limit);
  lastG = maxGForceLen / 9.81f;

  if (maxGForceLen > limitLen)
    return limitLen / maxGForceLen;

  return 1.f;
}

vec3 FighterJetBody::calcLift(vec3 right, float liftPower, float liftCoeff) const {
  vec3 liftVelocity = projectOnPlane(localVelocity, right);
  vec3 liftVelocityNorm = normalizeSafe(liftVelocity);
  float v2 = glm::length2(liftVelocity);

  float liftForce = v2 * liftCoeff * liftPower;
  vec3 liftDir = cross(liftVelocityNorm, right);
  vec3 lift = liftDir * liftForce;

  float dragForce = liftCoeff * liftCoeff * cfg.inducedDrag;
  vec3 dragDir = -liftVelocityNorm;
  vec3 finalInduceDrag = dragDir * v2 * dragForce;

  return lift + finalInduceDrag;
}

float FighterJetBody::calcSteering(float dt, float angularVelocity, float targetVelocity, float acc) const {
  float err = targetVelocity - angularVelocity;
  float a = acc * dt;

  return glm::clamp(err, -a, a);
}

void FighterJetBody::updateThrust() {
  rigidbody.addRelativeForce(cfg.throttle * cfg.maxThrust * localOrientation);
  cfg.throttle *= 0.9f;
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

  rigidbody.addRelativeForce(dragForceLocal);
}

void FighterJetBody::updateLift() {
  if (glm::length2(localVelocity) < 1.f)
    return;

  float flapsLiftPower = flapsDeployed * cfg.flapsLiftPower;
  float flapsAOABias = flapsDeployed * cfg.flapsAOABias;

  float flapsCoeff = getLiftCoeff(angleOfAttack + glm::radians(flapsAOABias));
  vec3 liftForce = calcLift({1.f, 0.f, 0.f}, cfg.liftPower + flapsLiftPower, flapsCoeff);

  float rudderCoeff = getLiftCoeffYaw(angleOfAttackYaw);
  vec3 liftForceYaw = calcLift({0.f, 1.f, 0.f}, cfg.rudderPower, rudderCoeff);

  rigidbody.addRelativeForce(liftForce);
  rigidbody.addRelativeForce(liftForceYaw);
}

void FighterJetBody::updateSteering(float dt) {
  float speed = getSpeed();
  float steeringPower = glm::clamp(speed / 20.f, 0.1f, 1.f);
  float gForceScaleing = calcGLimitter(controlInput, glm::radians(cfg.turnSpeed) * steeringPower);

  vec3 targetAV = controlInput * cfg.turnSpeed * steeringPower * gForceScaleing;
  vec3 av = glm::degrees(localAngularVelocity);

  vec3 correction {
    calcSteering(dt, av.x, targetAV.x, cfg.turnAcceleration.x * steeringPower),
    calcSteering(dt, av.y, targetAV.y, cfg.turnAcceleration.y * steeringPower),
    calcSteering(dt, av.z, targetAV.z, cfg.turnAcceleration.z * steeringPower),
  };

  vec3 aeroAngularDrag = -localAngularVelocity * speed * 0.5f;

  rigidbody.addRelativeTorqueInstantly(glm::radians(correction));
  rigidbody.addTorque(aeroAngularDrag);
}

void FighterJetBody::updateForceFromParts(float dt) {
  for (AircraftPart* part : parts) {
    vec3 bbCenterLocal = part->getLocalBoxCenter() * cfg.meshScale;
    vec3 bbSizeH = part->getLocalBoxSize() * 0.5f * cfg.meshScale; // Distance from bb center to its edges

    // Actual box center position
    vec3 bbRotatedCenterLocal = rigidbody.orientation * bbCenterLocal;
    vec3 worldPos = rigidbody.position + bbRotatedCenterLocal;

    vec3 corners[8] = {
      worldPos + rigidbody.orientation * bbSizeH,
      worldPos + rigidbody.orientation * vec3{-bbSizeH.x,  bbSizeH.y,  bbSizeH.z},
      worldPos + rigidbody.orientation * vec3{-bbSizeH.x, -bbSizeH.y,  bbSizeH.z},
      worldPos + rigidbody.orientation * vec3{-bbSizeH.x, -bbSizeH.y, -bbSizeH.z},
      worldPos + rigidbody.orientation * vec3{ bbSizeH.x, -bbSizeH.y,  bbSizeH.z},
      worldPos + rigidbody.orientation * vec3{ bbSizeH.x, -bbSizeH.y, -bbSizeH.z},
      worldPos + rigidbody.orientation * vec3{ bbSizeH.x,  bbSizeH.y, -bbSizeH.z},
      worldPos + rigidbody.orientation * vec3{-bbSizeH.x,  bbSizeH.y, -bbSizeH.z},
    };

    float lowestY = corners[0].y;
    vec3 contactCorner{};
    for (const vec3& corner : corners) {
      if (corner.y < lowestY) {
        lowestY = corner.y;
        contactCorner = corner;
      }
    }

    if (lowestY < cfg.groundHeight) {
      float depth = cfg.groundHeight - lowestY;
      // rigidbody.position.y += depth;

      vec3 r = contactCorner - rigidbody.position;
      vec3 f{};
      f.y = depth * cfg.stiffness;

      rigidbody.addForce(f);
      rigidbody.addTorque(cross(r, f));
    }
  }
}

void FighterJetBody::updateMesh(float dt) {
  animFlaps.rotate(flapsDeployed * 2.f - 1.f, dt);
  animAirbrake.rotate(airbrakeDeployed * 2.f - 1.f, dt);
  animPitch.rotate(controlInput.x, dt);
  animYaw.rotate(controlInput.y, dt);
  animRoll.rotate(controlInput.z, dt);

  leftFlap.localRotation = rightFlap.localRotation = animFlaps.currentRotation;
  airbrake.localRotation = animAirbrake.currentRotation;
  leftElevator.localRotation = rightElevator.localRotation = animPitch.currentRotation;
  leftRudder.localRotation = rightRudder.localRotation = animYaw.currentRotation;

  auto q = animRoll.currentRotation;
  leftAileron.localRotation = q;
  rightAileron.localRotation = {q.w, -q.x, q.y, q.z};

  animPitch.rotateBack(controlInput.x, 0.1f);
  animRoll.rotateBack(controlInput.z, 0.1f);
  animYaw.rotateBack(controlInput.y, 0.1f);

  mat4 bodyTransform = glm::mat4_cast(rigidbody.orientation * initialRotation);
  bodyTransform *= glm::scale(mat4(1.f), vec3(cfg.meshScale));

  for (AircraftPart* part : parts) {
    mat4 partTranslation = glm::translate(mat4(1.f), part->offset);
    mat4 partRotation = glm::mat4_cast(part->localRotation);

    part->modelRelative = bodyTransform * partTranslation * partRotation;

    vec3 size = part->getLocalBoxSize();
    vec3 center = part->getLocalBoxCenter();
    mat4 partBoxTranslation = glm::translate(mat4(1.f), center);
    mat4 partBoxScale = glm::scale(mat4(1.f), size * 0.5f);

    part->boxModelRelative = bodyTransform * partBoxTranslation * partRotation * partBoxScale;
  }
}

