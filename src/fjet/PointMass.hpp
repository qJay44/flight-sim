#pragma once

#include "glm/gtx/norm.hpp"

struct PointMass {
  float mass{1.f};
  float momentOfInertia = 1000.f;
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

  void addGravity(float g) {
    force.y += g * mass;
  }

  void applyForce(float dt) {
    vec3 acc = force / mass;
    velocity += acc * dt;
    position += velocity * dt;

    if (position.y < 0.f) {
      position.y = 0.f;
      velocity.y *= -0.1f;
    }

    angularVelocity += torque / momentOfInertia * dt;
    glm::quat rotStep = glm::quat(0.f, angularVelocity * dt);
    orientation += rotStep * 0.5f * orientation;
    orientation = glm::normalize(orientation);

    force *= 0.f;
    torque *= 0.f;
  }
};

