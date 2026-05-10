#include "FighterJetBody.hpp"

#include "PointMass.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/trigonometric.hpp"
#include "utils/types.hpp"
#include "../engine/mesh/fbx/model.hpp"

#include <algorithm>
#include <cmath>

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
  rigidbody.position.y = 10.f;
  rigidbody.localInertia = { 471906.f, 684784.f, 212878.f }; // TODO: Calculate at runtime (compile time?) based on mass
  rigidbody.drag = 0.02f;
  rigidbody.angularDrag = 0.5f;

  initialRotation = glm::angleAxis(PI, vec3{0.f, 1.f, 0.f});
}

const vec3& FighterJetBody::getPosition() const { return rigidbody.position; }
const vec3& FighterJetBody::getVelocity() const { return velocity; }
const glm::quat& FighterJetBody::getOrientation() const { return rigidbody.orientation; }

void FighterJetBody::update(float dt) {
  calcState(dt);
  calcAngleOfAttack();
  calcGForce(dt);

  updateThrust();
  updateDrag();
  updateLift();
  updateSteering(dt);
  updateForceFromParts(dt);

  rigidbody.addForce({0.f, -9.81f * rigidbody.mass, 0.f});
  rigidbody.update(dt);

  updateMesh(dt);

  controlInput *= 0.5f;
}

void FighterJetBody::draw(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  for (AircraftPart* part : parts) {
    shader.setUniformMatrix3f("u_localRotation", glm::mat3_cast(part->localRotation));
    part->draw(camera, shader, forceNoWireframe);
  }
}

void FighterJetBody::drawDebugMass(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  for (AircraftPart* part : parts)
    part->drawDebugMass(camera, shader, forceNoWireframe);
}

void FighterJetBody::drawDebugBoundaries(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  for (AircraftPart* part : parts)
    part->drawDebugBoundaries(camera, shader, forceNoWireframe);
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

void FighterJetBody::calcGForce(float dt) {
  auto invRotation = glm::conjugate(rigidbody.orientation);
  vec3 acc = (velocity - lastVelocity) / dt;
  localGForce = invRotation * acc;
  lastVelocity = velocity;
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
  float speed = glm::max(0.f, localVelocity.z);
  float steeringPower = glm::clamp((speed * speed) / 2e3f, 0.1f, 1.f);

  vec3 targetAV = controlInput * cfg.turnSpeed * steeringPower;
  vec3 av = glm::degrees(localAngularVelocity);

  vec3 correction {
    calcSteering(dt, av.x, targetAV.x, cfg.turnAcceleration.x * steeringPower),
    calcSteering(dt, av.y, targetAV.y, cfg.turnAcceleration.y * steeringPower),
    calcSteering(dt, av.z, targetAV.z, cfg.turnAcceleration.z * steeringPower),
  };

  rigidbody.addRelativeTorqueInstantly(glm::radians(correction));
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
      rigidbody.position.y += depth;

      vec3 r = contactCorner - rigidbody.position;
      vec3 force{};
      force.y = depth * cfg.stiffness;

      rigidbody.addForce(force);
      rigidbody.addTorque(cross(r, force));
    }
  }
}

void FighterJetBody::updateMesh(float dt) {
  // Flaps deploy animation
  {
    constexpr float maxAngle = glm::radians(-30.f);
    constexpr float rotSpeed = maxAngle;
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
    constexpr float rotSpeed = maxAngle;
    static float currentAngle = 0.f;

    float rotDir = airbrakeDeployed * 2.f - 1.f;
    currentAngle += rotSpeed * rotDir * dt;
    currentAngle = std::clamp(currentAngle, 0.f, maxAngle);

    auto q = glm::angleAxis(currentAngle, vec3(-1.f, 0.f, 0.f));
    airbrake.localRotation = q;
  }

  // Pitch animation
  {
    constexpr float maxAngle = glm::radians(20.f);
    constexpr float rotSpeed = maxAngle * 2.f;
    static float currentAngle = 0.f;

    float rotDir = controlInput.x;
    currentAngle += rotSpeed * rotDir * dt;
    currentAngle = std::clamp(currentAngle, -maxAngle, maxAngle);

    auto q = glm::angleAxis(currentAngle, vec3(-1.f, 0.f, 0.f));
    leftElevator.localRotation = rightElevator.localRotation = q;

    currentAngle -= rotSpeed * -rotDir * 0.1f + currentAngle * 0.1f;
  }

  // Roll animation
  {
    constexpr float maxAngle = glm::radians(20.f);
    constexpr float rotSpeed = maxAngle * 2.f;
    constexpr vec3 leftAileronHinge{0.906308f, 0.0f, -0.422618f}; // The rotated Left by 25 degrees around the Up
    static float currentAngle = 0.f;

    float rotDir = controlInput.z; // Clockwise roll if left aileron up (and right aileron is down (looking from the back))
    currentAngle += rotSpeed * rotDir * dt;
    currentAngle = std::clamp(currentAngle, -maxAngle, maxAngle);

    auto q = glm::angleAxis(currentAngle, leftAileronHinge);
    leftAileron.localRotation = q;
    rightAileron.localRotation = {q.w, -q.x, q.y, q.z};

    currentAngle -= rotSpeed * -rotDir * 0.1f + currentAngle * 0.1f;
  }

  // Yaw animation
  {
    constexpr float maxAngle = glm::radians(20.f);
    constexpr float rotSpeed = maxAngle * 2.f;
    static float currentAngle = 0.f;

    float rotDir = controlInput.y;
    currentAngle += rotSpeed * rotDir * dt;
    currentAngle = std::clamp(currentAngle, -maxAngle, maxAngle);

    auto q = glm::angleAxis(currentAngle, vec3(0.f, 1.f, 0.f));
    leftRudder.localRotation = rightRudder.localRotation = q;

    currentAngle -= rotSpeed * -rotDir * 0.1f + currentAngle * 0.1f;
  }

  mat4 bodyTransform = glm::translate(mat4(1.f), rigidbody.position);
  bodyTransform *= glm::mat4_cast(rigidbody.orientation * initialRotation);
  bodyTransform *= glm::scale(mat4(1.f), vec3(cfg.meshScale));

  for (AircraftPart* part : parts) {
    mat4 partTranslation = glm::translate(mat4(1.f), part->offset);
    mat4 partRotation = glm::mat4_cast(part->localRotation);

    part->model = bodyTransform * partTranslation * partRotation;

    vec3 size = part->getLocalBoxSize();
    vec3 center = part->getLocalBoxCenter();
    mat4 partBoxTranslation = glm::translate(mat4(1.f), center);
    mat4 partBoxScale = glm::scale(mat4(1.f), size * 0.5f);

    part->boxModel = bodyTransform * partBoxTranslation * partRotation * partBoxScale;
  }
}

