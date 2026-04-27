#pragma once

#include "AircraftPart.hpp"
#include "PointMass.hpp"

struct FighterJetBody {
  AircraftPart fuselage      {"Fuselage"};
  AircraftPart nose          {"Nose"};
  AircraftPart cockpit       {"Cockpit"};
  AircraftPart upperFuselage {"UpperFuselage"};
  AircraftPart engines       {"Engines"};
  AircraftPart wings         {"Wings"};
  AircraftPart leftAileron   {"LeftAileron"};
  AircraftPart rightAileron  {"RightAileron"};
  AircraftPart leftFlap      {"LeftFlap"};
  AircraftPart rightFlap     {"RightFlap"};
  AircraftPart leftElevator  {"LeftElevator"};
  AircraftPart rightElevator {"RightElevator"};
  AircraftPart rudders       {"Rudders"};
  AircraftPart leftRudder    {"LeftRudder"};
  AircraftPart rightRudder   {"RightRudder"};
  AircraftPart canopy        {"Canopy"};
  AircraftPart airbrake      {"Airbrake"};
  mat4 afterburner1; // left
  mat4 afterburner2; // right
  mat4 hardpoint1; // Weapon mount point (under left wing)
  mat4 hardpoint2; // Weapon mount point (under right wing)

  PointMass physicsCore;
  AircraftPart* allParts[17] = {
    &fuselage,
    &nose,
    &cockpit,
    &upperFuselage,
    &engines,
    &wings,
    &leftAileron,
    &rightAileron,
    &leftFlap,
    &rightFlap,
    &leftElevator,
    &rightElevator,
    &rudders,
    &leftRudder,
    &rightRudder,
    &canopy,
    &airbrake,
  };

  FighterJetBody(const fspath& fbxFilepath, float totalMass);

  void translateAll(vec3 v);
  void rotateAll(glm::quat q);

  void update(float dt);
  void updatePhysics(float dt);
  void updateMesh();
  void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;
  void drawDebug(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;
};

