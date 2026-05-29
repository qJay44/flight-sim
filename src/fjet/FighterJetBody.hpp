#pragma once

#include "AircraftPart.hpp"
#include "Animation.hpp"
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

  void update(float dt);

  void draw(const Camera* camera, Shader& shader) const;
  void drawDebugMass(const Camera* camera, Shader& shader) const;
  void drawDebugBoundaries(const Camera* camera, Shader& shader) const;

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
  AircraftPart* parts[17] = {
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
  glm::quat initialRotation;

  vec3 velocity{};
  vec3 lastVelocity{};
  vec3 localVelocity{};
  vec3 localAngularVelocity{};
  vec3 localGForce{1.f};
  vec3 controlInput{}; // pitch, yaw, roll; [-1, 1]

  float angleOfAttack = glm::radians(10.f);
  float angleOfAttackYaw = 0.f;

  // Drag
  float Cd_forward = 0.02f;
  float Cd_side = 0.50f;
  float Cd_vertical = 0.80f;

  struct Config {
    float throttle = 0.f;
    float maxThrust = 1.f;
    float groundHeight = 0.f;
    float stiffness = 100000.f;
    float airbrakeDrag = 1.f;
    float flapsDrag = 2.f;
    float flapsLiftPower = 0.5f;
    float flapsAOABias = 10.f;
    float meshScale = 1.f;
    float inducedDrag = 10.f;
    float liftPower = 1.f;
    float rudderPower = 1.f;
    vec3 turnSpeed{};
    vec3 turnAcceleration{};
  } cfg;

  Animation animFlaps;
  Animation animAirbrake;
  Animation animPitch;
  Animation animRoll;
  Animation animYaw;

  bool airbrakeDeployed = false;
  bool flapsDeployed = false;

private:
  static float getLiftCoeff(float angleRad);
  static float getLiftCoeffYaw(float angleRad);

  void calcState(float dt);
  void calcAngleOfAttack();
  void calcGForce(float dt);
  vec3 calcLift(vec3 right, float liftPower, float liftCoeff) const;
  float calcSteering(float dt, float angularVelocity, float targetVelocity, float acc) const;

  void updateThrust();
  void updateDrag();
  void updateLift();
  void updateSteering(float dt);
  void updateForceFromParts(float dt);
  void updateMesh(float dt);
};

