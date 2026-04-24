#include "FighterJetMesh.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "utils/types.hpp"
#include "../engine/mesh/fbx/model.hpp"

FighterJetMesh::FighterJetMesh(const fspath& fbxFilepath) {
  fbx::Model model = fbx::load(fbxFilepath);
  std::unordered_map<std::string, Mesh> meshMap;
  std::unordered_map<std::string, mat4> socketMap;

  for (fbx::NamedMesh& nmesh : model.meshes)
    meshMap[nmesh.name] = std::move(nmesh.mesh);

  fuselage      = std::move(meshMap["Fuselage"]);
  nose          = std::move(meshMap["Nose"]);
  cockpit       = std::move(meshMap["Cockpit"]);
  upperFuselage = std::move(meshMap["UpperFuselage"]);
  engines       = std::move(meshMap["Engines"]);
  wings         = std::move(meshMap["Wings"]);
  leftAileron   = std::move(meshMap["LeftAileron"]);
  rightAileron  = std::move(meshMap["RightAileron"]);
  leftFlap      = std::move(meshMap["LeftFlap"]);
  rightFlap     = std::move(meshMap["RightFlap"]);
  leftElevator  = std::move(meshMap["LeftElevator"]);
  rightElevator = std::move(meshMap["RightElevator"]);
  rudders       = std::move(meshMap["Rudders"]);
  leftRudder    = std::move(meshMap["LeftRudder"]);
  rightRudder   = std::move(meshMap["RightRudder"]);
  canopy        = std::move(meshMap["Canopy"]);
  airbrake      = std::move(meshMap["Airbrake"]);

  for (fbx::Socket& s : model.sockets)
    socketMap[s.name] = s.transform;

  afterburner1 = socketMap["Afterburner1"];
  afterburner2 = socketMap["Afterburner2"];
  hardpoint1   = socketMap["Hardpoint1"];
  hardpoint2   = socketMap["Hardpoint2"];
}

void FighterJetMesh::translateAll(vec3 v) {
  fuselage     .translate(v);
  nose         .translate(v);
  cockpit      .translate(v);
  upperFuselage.translate(v);
  engines      .translate(v);
  wings        .translate(v);
  leftAileron  .translate(v);
  rightAileron .translate(v);
  leftFlap     .translate(v);
  rightFlap    .translate(v);
  leftElevator .translate(v);
  rightElevator.translate(v);
  rudders      .translate(v);
  leftRudder   .translate(v);
  rightRudder  .translate(v);
  canopy       .translate(v);
  airbrake     .translate(v);

  afterburner1 = glm::translate(afterburner1, v);
  afterburner2 = glm::translate(afterburner2, v);
  hardpoint1   = glm::translate(hardpoint1  , v);
  hardpoint2   = glm::translate(hardpoint2  , v);
}

void FighterJetMesh::rotateAll(glm::quat q) {
  fuselage     .rotate(q);
  nose         .rotate(q);
  cockpit      .rotate(q);
  upperFuselage.rotate(q);
  engines      .rotate(q);
  wings        .rotate(q);
  leftAileron  .rotate(q);
  rightAileron .rotate(q);
  leftFlap     .rotate(q);
  rightFlap    .rotate(q);
  leftElevator .rotate(q);
  rightElevator.rotate(q);
  rudders      .rotate(q);
  leftRudder   .rotate(q);
  rightRudder  .rotate(q);
  canopy       .rotate(q);
  airbrake     .rotate(q);

  mat4 rot = glm::mat4_cast(q);
  afterburner1 = afterburner1 * rot;
  afterburner2 = afterburner2 * rot;
  hardpoint1   = hardpoint1   * rot;
  hardpoint2   = hardpoint2   * rot;
}

void FighterJetMesh::draw(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  shader.setUniform3f("u_color", vec3(1.f));
  canopy.draw(camera, shader, forceNoWireframe);

  shader.setUniform3f("u_color", {0.24377f, 0.355047f, 0.6226415f});
  upperFuselage.draw(camera, shader, forceNoWireframe);
  fuselage     .draw(camera, shader, forceNoWireframe);
  nose         .draw(camera, shader, forceNoWireframe);
  cockpit      .draw(camera, shader, forceNoWireframe);
  engines      .draw(camera, shader, forceNoWireframe);
  wings        .draw(camera, shader, forceNoWireframe);
  leftAileron  .draw(camera, shader, forceNoWireframe);
  rightAileron .draw(camera, shader, forceNoWireframe);
  leftFlap     .draw(camera, shader, forceNoWireframe);
  rightFlap    .draw(camera, shader, forceNoWireframe);
  leftElevator .draw(camera, shader, forceNoWireframe);
  rightElevator.draw(camera, shader, forceNoWireframe);
  rudders      .draw(camera, shader, forceNoWireframe);
  leftRudder   .draw(camera, shader, forceNoWireframe);
  rightRudder  .draw(camera, shader, forceNoWireframe);
  airbrake     .draw(camera, shader, forceNoWireframe);
}

