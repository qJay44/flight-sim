#pragma once

#include "../components/PhysicsComponent.hpp"

namespace ecs::system::PhysicsSystem {

using namespace ecs::component;

void addForce(PhysicsComponent& c, vec3 f);
void addRelativeForce(PhysicsComponent& c, vec3 f);
void addTorque(PhysicsComponent& c, vec3 t);
void addRelativeTorque(PhysicsComponent& c, vec3 t);
void addRelativeTorqueInstantly(PhysicsComponent& c, vec3 t);
void applyDamping(PhysicsComponent& c, float dt);
void update(entt::registry& registry, float dt);

} // namespace ecs::system::PhysicsSystem

