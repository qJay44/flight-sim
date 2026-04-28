#pragma once

#include "glm/ext/quaternion_common.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/gtx/fast_trigonometry.hpp"
#include "../engine/mesh/Transformable.hpp"

struct PointMass : public Transformable {
  float mass{1.f};
  float momentOfInertia = 1000.f;
  vec3 position{};
  vec3 velocity{};
  vec3 lastVelocity{};
  vec3 localVelocity{};
  vec3 angularVelocity{};
  vec3 localAngularVelocity{};
  vec3 localGForce{1.f};
  vec3 localThustDir{0.f, 0.f, -1.f}; // NOTE: The plane looks at -Z initially;
  glm::quat orientation{};

  vec3 force{};
  vec3 torque{};

  float angleOfAttack = 0.f;
  float angleOfAttackYaw = 0.f;

  void applyThrust(float thrust) {
    vec3 worldThrustDir = orientation * localThustDir;
    force += worldThrustDir * thrust;
  }

  void applyGravity() {
    force.y += -9.81f * mass;
  }

  void calcState(float dt) {
    auto invRotation = glm::inverse(glm::quat_cast(matRotation));
    localVelocity = invRotation * velocity;
    localAngularVelocity = invRotation * angularVelocity;
  }

  void calcAngleOfAttack() {
    if (glm::length2(localVelocity) < 0.1f) {
      angleOfAttack = 0.f;
      angleOfAttackYaw = 0.f;
      return;
    }

    angleOfAttack = atan2(-localVelocity.y, localVelocity.z);
    angleOfAttackYaw = atan2(localVelocity.x, localVelocity.z);
  }

  void calcGForce(float dt) {
    auto invRotation = glm::inverse(glm::quat_cast(matRotation));
    vec3 acc = (velocity - lastVelocity) / dt;
    localGForce = invRotation * acc;
    lastVelocity = velocity;
  }

  void update(float dt) {
    vec3 acc = force / mass;
    velocity += acc * dt;
    position += velocity * dt;

    angularVelocity += torque / momentOfInertia * dt;
    glm::quat rotStep = glm::quat(0.f, angularVelocity * dt);
    orientation += rotStep * 0.5f * orientation;
    orientation = glm::normalize(orientation);

    force *= 0.f;
    torque *= 0.f;
  }
};

