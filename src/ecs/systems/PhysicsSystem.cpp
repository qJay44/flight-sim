#include "PhysicsSystem.hpp"

#include "glm/gtx/matrix_operation.hpp"
#include "../components/TransformComponent.hpp"

namespace ecs::system::PhysicsSystem {

void addForce(PhysicsComponent& c, vec3 f) {
  c.force += f;
}

void addRelativeForce(PhysicsComponent& c, vec3 f) {
  c.force += c.orientation * f;
}

void addTorque(PhysicsComponent& c, vec3 t) {
  c.torque += t;
}

void addRelativeTorque(PhysicsComponent& c, vec3 t) {
  c.torque += c.orientation * t;
}

void addRelativeTorqueInstantly(PhysicsComponent& c, vec3 t) {
  c.angularVelocity += c.orientation * t;
}

void applyDamping(PhysicsComponent& c, float dt) {
  c.velocity *= 1.f / (1.f + c.drag * dt);

  float speedFatctor = glm::length(c.velocity) * 0.01f;
  float effectiveAnularDrag = glm::clamp(0.1f / (1.f + speedFatctor), 0.01f, 0.99f);
  c.angularVelocity *= glm::pow(effectiveAnularDrag, dt);
}

void update(entt::registry& registry, float dt) {
  for (auto entity : registry.view<PhysicsComponent, TransformComponent>()) {
    auto& phyComponent = registry.get<PhysicsComponent>(entity);
    auto& transComponent = registry.get<TransformComponent>(entity);

    mat3 R = glm::mat3_cast(phyComponent.orientation);
    mat3 invIntertiaWorld = R * glm::diagonal3x3(1.f / phyComponent.localInertia) * glm::transpose(R);
    vec3 angularAcc = invIntertiaWorld * phyComponent.torque;

    vec3 acc = phyComponent.force * phyComponent.inverseMass;
    phyComponent.velocity += acc * dt;
    transComponent.position += phyComponent.velocity * dt;

    phyComponent.angularVelocity += angularAcc * dt;

    glm::quat rotStep = glm::quat(0.f, phyComponent.angularVelocity * dt);
    phyComponent.orientation += rotStep * 0.5f * phyComponent.orientation;
    phyComponent.orientation = glm::normalize(phyComponent.orientation);

    phyComponent.force *= 0.f;
    phyComponent.torque *= 0.f;

    applyDamping(phyComponent, dt);
  }
}

} // namespace ecs::system::PhysicsSystem

