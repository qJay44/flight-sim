#pragma once

namespace ecs::component {

struct PhysicsComponent {
  float mass = 1.f;
  float inverseMass = 1.f;
  float drag = 1.f;
  vec3 localInertia{1.f, 1.f, 1.f};
  vec3 velocity{};
  vec3 angularVelocity{};
  glm::quat orientation{};

  vec3 force{};
  vec3 torque{};
};

} // namespace ecs::component

