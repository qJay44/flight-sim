#include "FighterJetBody.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "utils/types.hpp"
#include "../engine/mesh/fbx/model.hpp"

FighterJetBody::FighterJetBody(const fspath& fbxFilepath, float totalMass) {
  fbx::Model model = fbx::load(fbxFilepath);
  std::unordered_map<std::string, fbx::NamedMesh> meshMap;
  std::unordered_map<std::string, mat4> socketMap;

  for (fbx::NamedMesh& nmesh : model.meshes)
    meshMap[nmesh.name] = std::move(nmesh);

  fuselage     .mesh = std::move(meshMap["Fuselage"].mesh);
  nose         .mesh = std::move(meshMap["Nose"].mesh);
  cockpit      .mesh = std::move(meshMap["Cockpit"].mesh);
  upperFuselage.mesh = std::move(meshMap["UpperFuselage"].mesh);
  engines      .mesh = std::move(meshMap["Engines"].mesh);
  wings        .mesh = std::move(meshMap["Wings"].mesh);
  leftAileron  .mesh = std::move(meshMap["LeftAileron"].mesh);
  rightAileron .mesh = std::move(meshMap["RightAileron"].mesh);
  leftFlap     .mesh = std::move(meshMap["LeftFlap"].mesh);
  rightFlap    .mesh = std::move(meshMap["RightFlap"].mesh);
  leftElevator .mesh = std::move(meshMap["LeftElevator"].mesh);
  rightElevator.mesh = std::move(meshMap["RightElevator"].mesh);
  rudders      .mesh = std::move(meshMap["Rudders"].mesh);
  leftRudder   .mesh = std::move(meshMap["LeftRudder"].mesh);
  rightRudder  .mesh = std::move(meshMap["RightRudder"].mesh);
  canopy       .mesh = std::move(meshMap["Canopy"].mesh);
  airbrake     .mesh = std::move(meshMap["Airbrake"].mesh);

  float finalTotalMass = 0.f;

  // NOTE: Total multipler goes above 1.0 (> 100%)
  finalTotalMass += fuselage     .mass = totalMass * 0.18f;
  finalTotalMass += nose         .mass = totalMass * 0.04f;
  finalTotalMass += cockpit      .mass = totalMass * 0.07f;
  finalTotalMass += upperFuselage.mass = totalMass * 0.18f;
  finalTotalMass += engines      .mass = totalMass * 0.20f;
  finalTotalMass += wings        .mass = totalMass * 0.14f;
  finalTotalMass += leftAileron  .mass = totalMass * 0.01f;
  finalTotalMass += rightAileron .mass = totalMass * 0.01f;
  finalTotalMass += leftFlap     .mass = totalMass * 0.01f;
  finalTotalMass += rightFlap    .mass = totalMass * 0.01f;
  finalTotalMass += leftElevator .mass = totalMass * 0.01f;
  finalTotalMass += rightElevator.mass = totalMass * 0.01f;
  finalTotalMass += rudders      .mass = totalMass * 0.04f;
  finalTotalMass += leftRudder   .mass = totalMass * 0.02f;
  finalTotalMass += rightRudder  .mass = totalMass * 0.02f;
  finalTotalMass += canopy       .mass = totalMass * 0.07f;
  finalTotalMass += airbrake     .mass = totalMass * 0.01f;

  // Assuming aircraft's center of mass at (0, 0, 0)
  fuselage     .offset = meshMap["Fuselage"].averagePos;
  nose         .offset = meshMap["Nose"].averagePos;
  cockpit      .offset = meshMap["Cockpit"].averagePos;
  upperFuselage.offset = meshMap["UpperFuselage"].averagePos;
  engines      .offset = meshMap["Engines"].averagePos;
  wings        .offset = meshMap["Wings"].averagePos;
  leftAileron  .offset = meshMap["LeftAileron"].averagePos;
  rightAileron .offset = meshMap["RightAileron"].averagePos;
  leftFlap     .offset = meshMap["LeftFlap"].averagePos;
  rightFlap    .offset = meshMap["RightFlap"].averagePos;
  leftElevator .offset = meshMap["LeftElevator"].averagePos;
  rightElevator.offset = meshMap["RightElevator"].averagePos;
  rudders      .offset = meshMap["Rudders"].averagePos;
  leftRudder   .offset = meshMap["LeftRudder"].averagePos;
  rightRudder  .offset = meshMap["RightRudder"].averagePos;
  canopy       .offset = meshMap["Canopy"].averagePos;
  airbrake     .offset = meshMap["Airbrake"].averagePos;

  canopy.color = vec3(1.f);

  for (fbx::Socket& s : model.sockets)
    socketMap[s.name] = s.transform;

  afterburner1 = socketMap["Afterburner1"];
  afterburner2 = socketMap["Afterburner2"];
  hardpoint1   = socketMap["Hardpoint1"];
  hardpoint2   = socketMap["Hardpoint2"];

  vec3 weightedPos{};
  for (AircraftPart* part : allParts)
    weightedPos += part->mass * part->offset;

  physicsCore.position = weightedPos / finalTotalMass;
  physicsCore.mass = finalTotalMass;
}

void FighterJetBody::translateAll(vec3 v) {
  for (AircraftPart* part : allParts)
    part->mesh.translate(v);

  afterburner1 = glm::translate(afterburner1, v);
  afterburner2 = glm::translate(afterburner2, v);
  hardpoint1   = glm::translate(hardpoint1  , v);
  hardpoint2   = glm::translate(hardpoint2  , v);
}

void FighterJetBody::rotateAll(glm::quat q) {
  for (AircraftPart* part : allParts)
    part->mesh.rotate(q);

  mat4 rot = glm::mat4_cast(q);
  afterburner1 = afterburner1 * rot;
  afterburner2 = afterburner2 * rot;
  hardpoint1   = hardpoint1   * rot;
  hardpoint2   = hardpoint2   * rot;
}

void FighterJetBody::update(float dt) {
  float dtSub = dt * 0.1f;
  for (size_t i = 0; i < 10; i++) {
    updatePhysics(dtSub);
  }
  updateMesh();
}

void FighterJetBody::updatePhysics(float dt) {
  vec3 totalForce{0.f, -9.81f * physicsCore.mass, 0.f};
  vec3 totalTorque{};

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
      totalForce += force;
      totalTorque += cross(rotatedOffset, force);
    }
  }
  physicsCore.velocity += (totalForce / physicsCore.mass) * dt;
  physicsCore.position += physicsCore.velocity * dt;

  constexpr float momentOfInertia = 1000.f;
  physicsCore.angularVelocity += (totalTorque / momentOfInertia) * dt;

  glm::quat rotStep = glm::quat(0.f, physicsCore.angularVelocity * dt);
  physicsCore.orientation += rotStep * 0.5f * physicsCore.orientation;
  physicsCore.orientation = glm::normalize(physicsCore.orientation);
}

void FighterJetBody::updateMesh() {
  mat4 bodyRot = glm::mat4_cast(physicsCore.orientation);

  for (AircraftPart* part : allParts) {
    vec3 partWorldPos = physicsCore.position;
    part->mesh.setTranslationMat(partWorldPos);
    part->mesh.setRotationMat(bodyRot);

    part->debugMassMesh.setTranslationMat(partWorldPos + physicsCore.orientation * part->offset);
    part->debugMassMesh.setRotationMat(bodyRot);
    part->debugMassMesh.setScale(physicsCore.mass / part->mass);
  }
}

void FighterJetBody::draw(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  for (AircraftPart* part : allParts)
    part->draw(camera, shader, forceNoWireframe);
}

void FighterJetBody::drawDebug(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  for (AircraftPart* part : allParts)
    part->drawDebug(camera, shader, forceNoWireframe);
}

