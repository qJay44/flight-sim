#include "FighterJetBody.hpp"

#include "glm/gtc/quaternion.hpp"
#include "utils/types.hpp"
#include "../engine/mesh/fbx/model.hpp"

FighterJetBody::FighterJetBody(const fspath& fbxFilepath, float totalMass) {
  fbx::Model model = fbx::load(fbxFilepath);
  std::unordered_map<std::string, fbx::UfbxMesh> meshMap;
  std::unordered_map<std::string, mat4> socketMap;

  for (fbx::UfbxMesh& nmesh : model.meshes)
    meshMap[nmesh.name] = std::move(nmesh);

  vec3 weightedPos{};

  for (AircraftPart* part : allParts) {
    auto it = meshMap.find(part->name);
    if (it == meshMap.end())
      error("[FighterJetBody::FighterJetBody] Didn't find [{}] in map", part->name);

    part->mesh = std::move(it->second.mesh);
    part->mass = totalMass * part->massPercent;
    part->offset = it->second.averagePos;
    weightedPos += part->mass * part->offset;
  }

  canopy.color = vec3(1.f);

  for (fbx::Socket& s : model.sockets)
    socketMap[s.name] = s.transform;

  afterburner1 = socketMap["Afterburner1"];
  afterburner2 = socketMap["Afterburner2"];
  hardpoint1   = socketMap["Hardpoint1"];
  hardpoint2   = socketMap["Hardpoint2"];

  physicsCore.position = weightedPos / totalMass;
  physicsCore.mass = totalMass;
}

const vec3& FighterJetBody::getPosition() const { return physicsCore.position; }
const glm::quat& FighterJetBody::getOrientaion() const { return physicsCore.orientation; }

void FighterJetBody::applyThrust(float thrust) {
  physicsCore.applyThrust(thrust);
}

void FighterJetBody::update(float dt) {
  physicsCore.calcState(dt);
  physicsCore.calcAngleOfAttack();
  physicsCore.calcGForce(dt);

  float dtSub = dt * 0.1f;
  for (size_t i = 0; i < 10; i++)
    updatePhysics(dtSub);

  updateMesh();
}

void FighterJetBody::draw(const Camera* camera, Shader& shader, bool forceNoWireframe) const {

  for (AircraftPart* part : allParts)
    part->draw(camera, shader, forceNoWireframe);
}

void FighterJetBody::drawDebug(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  for (AircraftPart* part : allParts)
    part->drawDebug(camera, shader, forceNoWireframe);
}

void FighterJetBody::updatePhysics(float dt) {
  physicsCore.applyGravity();
  physicsCore.applyDrag(airbrakeDrag * airbrakeDeployed, flapsDrag * flapsDeployed);

  constexpr float groundHeight = 0.f;
  constexpr float stiffness = 100000.f;
  constexpr float dampingCoeff = 5000.f;

  for (AircraftPart* part : allParts) {
    vec3 rotatedOffset = physicsCore.orientation * part->offset;
    vec3 worldPos = physicsCore.position + rotatedOffset;

    if (worldPos.y < groundHeight) {
      float depth = groundHeight - worldPos.y;
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

void FighterJetBody::updateMesh() {
  mat4 bodyRot = glm::mat4_cast(physicsCore.orientation);

  for (AircraftPart* part : allParts) {
    vec3 rotatedOffset = physicsCore.orientation * part->offset;
    vec3 partWorldPos = physicsCore.position + rotatedOffset;
    auto totalRotation = physicsCore.orientation * part->localRotation;

    part->mesh.setMatTranslation(partWorldPos);
    part->mesh.setMatRotation(totalRotation);

    part->debugMassMesh.setMatTranslation(partWorldPos);
    part->debugMassMesh.setMatRotation(bodyRot);
    part->debugMassMesh.setMatScale(physicsCore.mass / part->mass);
  }
}

