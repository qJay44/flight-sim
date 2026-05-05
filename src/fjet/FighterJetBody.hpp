#pragma once

#include "AircraftPart.hpp"
#include "PointMass.hpp"
#include "MassConfig.hpp"
#include "glm/trigonometric.hpp"

class FighterJetBody {
public:
  FighterJetBody(const fspath& fbxFilepath, vec3 orientation, float totalMass);

  const vec3& getPosition() const;
  const vec3& getVelocity() const;
  const glm::quat& getOrientation() const;
  const float& getMaxThrust() const;

  void setMaxThrust(float t);
  void setStiffness(float s);
  void setDampingCoeff(float c);

  void toggleAirbrake();
  void toggleFlaps();
  void addThrottle(float normalizedValue); // [0, 1]

  void update(float dt);

  void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;
  void drawDebug(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;

private:
  friend struct FighterJet;
  friend struct gui;

  AircraftPart fuselage      {"Fuselage"     , MassConfig::fuselage     };
  AircraftPart nose          {"Nose"         , MassConfig::nose         };
  AircraftPart cockpit       {"Cockpit"      , MassConfig::cockpit      };
  AircraftPart upperFuselage {"UpperFuselage", MassConfig::upperFuselage};
  AircraftPart engines       {"Engines"      , MassConfig::engines      };
  AircraftPart wings         {"Wings"        , MassConfig::wings        };
  AircraftPart leftAileron   {"LeftAileron"  , MassConfig::leftAileron  };
  AircraftPart rightAileron  {"RightAileron" , MassConfig::rightAileron };
  AircraftPart leftFlap      {"LeftFlap"     , MassConfig::leftFlap     };
  AircraftPart rightFlap     {"RightFlap"    , MassConfig::rightFlap    };
  AircraftPart leftElevator  {"LeftElevator" , MassConfig::leftElevator };
  AircraftPart rightElevator {"RightElevator", MassConfig::rightElevator};
  AircraftPart rudders       {"Rudders"      , MassConfig::rudders      };
  AircraftPart leftRudder    {"LeftRudder"   , MassConfig::leftRudder   };
  AircraftPart rightRudder   {"RightRudder"  , MassConfig::rightRudder  };
  AircraftPart canopy        {"Canopy"       , MassConfig::canopy       };
  AircraftPart airbrake      {"Airbrake"     , MassConfig::airbrake     };
  mat4 afterburner1; // left
  mat4 afterburner2; // right
  mat4 hardpoint1; // Weapon mount point (under left wing)
  mat4 hardpoint2; // Weapon mount point (under right wing)

  PointMass rigidbody;
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

  vec3 localOrientation;

  vec3 velocity{};
  vec3 lastVelocity{};
  vec3 localVelocity{};
  vec3 localAngularVelocity{};
  vec3 localGForce{1.f};

  float angleOfAttack = glm::radians(10.f);
  float angleOfAttackYaw = 0.f;

  // Drag
  float Cd_forward = 0.02f;
  float Cd_side = 0.50f;
  float Cd_vertical = 0.80f;

  float throttle = 0.f;
  float maxThrust = 1.f;
  float groundHeight = 0.f;
  float stiffness = 100000.f;
  float dampingCoeff = 5000.f;
  float airbrakeDrag = 0.f;
  float flapsDrag = 2.f;
  float flapsLiftPower = 0.5f;
  float flapsAOABias = 10.f;
  float meshScale = 1.f;
  float inducedDrag = 100.f;
  float liftPower = 1.f;
  float rudderPower = 1.f;

  bool airbrakeDeployed = false;
  bool flapsDeployed = false;

private:
  static float getLiftCoeff(float angleRad);
  static float getLiftCoeffYaw(float angleRad);

  void calcState(float dt);
  void calcAngleOfAttack();
  void calcGForce(float dt);
  vec3 calcLift(vec3 right, float liftPower, float liftCoeff);

  void updateThrust();
  void updateDrag();
  void updateLift();
  void updateForceFromParts(float dt);
  void updateMesh(float dt);
};

