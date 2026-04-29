#pragma once

#include "AircraftPart.hpp"
#include "PointMass.hpp"
#include "MassConfig.hpp"

class FighterJetBody {
public:
  FighterJetBody(const fspath& fbxFilepath, float totalMass);

  const vec3& getPosition() const;
  const glm::quat& getOrientaion() const;
  const float& getMaxThrust() const;

  void setMaxThrust(float t);
  void setStiffness(float s);
  void setDampingCoeff(float c);

  void toggleAirbrake();
  void toggleFlaps();
  void applyThrust(float normalizedValue); // [0, 1]

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

  float maxThrust = 1.f;
  float groundHeight = 0.f;
  float stiffness = 100000.f;
  float dampingCoeff = 5000.f;
  float airbrakeDrag = 0.f;
  float flapsDrag = 0.f;
  float meshScale = 1.f;

  bool airbrakeDeployed = false;
  bool flapsDeployed = false;

private:
  void updatePhysics(float dt);
  void updateMesh(float dt);
};

