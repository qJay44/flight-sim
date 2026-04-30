#include "FighterJetBody.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/trigonometric.hpp"
#include "utils/types.hpp"
#include "../engine/mesh/fbx/model.hpp"
#include <algorithm>

FighterJetBody::FighterJetBody(const fspath& fbxFilepath, float totalMass) {
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

  physicsCore.mass = totalMass;
  physicsCore.position.y = 20.f;
}

const vec3& FighterJetBody::getPosition() const { return physicsCore.position; }
const glm::quat& FighterJetBody::getOrientaion() const { return physicsCore.orientation; }
const float& FighterJetBody::getMaxThrust() const { return maxThrust; }

void FighterJetBody::setMaxThrust(float t) { maxThrust = t; }
void FighterJetBody::setStiffness(float s) { stiffness = s; }
void FighterJetBody::setDampingCoeff(float c) { dampingCoeff = c; }

void FighterJetBody::toggleAirbrake() { airbrakeDeployed = !airbrakeDeployed; }
void FighterJetBody::toggleFlaps() { flapsDeployed = !flapsDeployed; }

void FighterJetBody::applyThrust(float input) {
  physicsCore.applyThrust(input * maxThrust);
}

void FighterJetBody::update(float dt) {
  physicsCore.calcState(dt);
  physicsCore.calcAngleOfAttack();
  physicsCore.calcGForce(dt);

  updatePhysics(dt);
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

void FighterJetBody::updatePhysics(float dt) {
  physicsCore.applyGravity();
  physicsCore.applyDrag(airbrakeDrag * airbrakeDeployed, flapsDrag * flapsDeployed);

  for (AircraftPart* part : allParts) {
    vec3 rotatedOffset = physicsCore.orientation * part->offset;
    vec3 worldPos = physicsCore.position;

    if (worldPos.y < groundHeight) {
      float depth = glm::clamp(groundHeight - worldPos.y, 0.f, 0.5f);
      float spring = depth * stiffness;

      vec3 partVel = physicsCore.velocity + cross(physicsCore.angularVelocity, rotatedOffset);
      float damping = -partVel.y * dampingCoeff;

      vec3 force{0.f, spring + damping, 0.f};
      physicsCore.force += force;
      physicsCore.torque += cross(rotatedOffset, force);
    }
  }
  physicsCore.update(dt);
}

void FighterJetBody::updateMesh(float dt) {
  // Flaps deploy animation
  {
    constexpr float maxAngle = glm::radians(30.f);
    constexpr float rotSpeed = maxAngle * 0.5f;
    static float currentAngle = 0.f;

    float rotDir = flapsDeployed * 2.f - 1.f;
    currentAngle += rotSpeed * rotDir * dt;
    currentAngle = std::clamp(currentAngle, 0.f, maxAngle);

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

  mat4 bodyTransform = glm::translate(mat4(1.f), physicsCore.position);
  bodyTransform *= glm::mat4_cast(physicsCore.orientation);
  bodyTransform *= glm::scale(mat4(1.f), vec3(meshScale));

  for (AircraftPart* part : allParts) {
    mat4 partMove = glm::translate(mat4(1.f), part->offset);
    partMove *= glm::mat4_cast(part->localRotation);

    part->model = bodyTransform * partMove;
  }
}

