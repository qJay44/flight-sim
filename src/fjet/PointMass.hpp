#pragma once

#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/gtx/matrix_operation.hpp"
#include "glm/matrix.hpp"

struct PointMass {
  float mass = 1.f;
  float inverseMass = 1.f;
  float drag = 1.f;
  float angularDrag= 1.f;
  vec3 localInertia{1.f, 1.f, 1.f};
  vec3 position{};
  vec3 velocity{};
  vec3 angularVelocity{};
  glm::quat orientation{};

  vec3 force{};
  vec3 torque{};

  void addForce(vec3 f) {
    force += f;
  }

  void addRelativeForce(vec3 f) {
    force += orientation * f;
  }

  void addTorque(vec3 t) {
    torque += t;
  }

  void addRelativeTorqueInstantly(vec3 t) {
    angularVelocity += orientation * t;
  }

  void applyDamping(float dt) {
    velocity *= 1.f / (1.f + drag * dt);
    angularVelocity *= 1.f / (1.f + angularDrag * dt);
  }

  void update(float dt) {
    mat3 R = glm::mat3_cast(orientation);
    mat3 invIntertiaWorld = R * glm::diagonal3x3(1.f / localInertia) * glm::transpose(R);
    vec3 angularAcc = invIntertiaWorld * torque;

    vec3 acc = force * inverseMass;
    velocity += acc * dt;
    position += velocity * dt;

    if (position.y < 0.f) {
      position.y = 0.f;
      velocity.y *= -0.1f;
    }

    angularVelocity += angularAcc * dt;

    glm::quat rotStep = glm::quat(0.f, angularVelocity * dt);
    orientation += rotStep * 0.5f * orientation;
    orientation = glm::normalize(orientation);

    force *= 0.f;
    torque *= 0.f;

    applyDamping(dt);
  }
};

