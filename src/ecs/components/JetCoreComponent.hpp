#pragma once

#include "glm/gtc/quaternion.hpp"
#include "../../core/Animation.hpp"

namespace ecs::component {

struct JetCoreComponent {
  std::array<entt::entity, 17> partEntities{};

  vec3 localOrientation;
  vec3 localUp;
  glm::quat initialRotation{};

  vec3 localVelocity{};
  vec3 localAngularVelocity{};
  vec3 localGForce{1.f};
  vec3 controlInput{}; // pitch, yaw, roll; [-1, 1]

  float camDistance = 10.f;
  float camFollowSpeed = 25.f;
  float camLookAheadDistance = 2.f;

  float angleOfAttack = glm::radians(10.f);
  float angleOfAttackYaw = 0.f;

  // Drag
  float Cd_forward = 0.02f;
  float Cd_side = 0.50f;
  float Cd_vertical = 0.80f;

  float throttle = 0.f;
  float maxThrust = 1.f;
  float stiffness = 100000.f;
  float airbrakeDrag = 1.f;
  float flapsDrag = 2.f;
  float flapsLiftPower = 0.5f;
  float flapsAOABias = 10.f;
  float inducedDrag = 10.f;
  float liftPower = 1.f;
  float rudderPower = 1.f;
  float gLimit = 8.f;
  float gLimitPitch = -4.f;
  vec3 turnSpeed{};
  vec3 turnAcceleration{};

  core::Animation animFlaps{};
  core::Animation animAirbrake{};
  core::Animation animPitch{};
  core::Animation animRoll{};
  core::Animation animYaw{};

  bool airbrakeDeployed = false;
  bool flapsDeployed = false;

  float lastG = 0.f;
};

} // namespace ecs::component

