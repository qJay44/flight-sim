#pragma once

#include "global.hpp"
#include "glm/gtc/quaternion.hpp"
#include "../engine/mesh/Transformable.hpp"

struct PointMass : public Transformable {
  vec3 position{};
  vec3 velocity{};
  vec3 angularVelocity{};
  vec3 force{};
  glm::quat orientation{};
  float mass{1.f};

  // void update() {
  //   vec3 acc = force / mass;
  //   velocity += acc * global::dt;
  //   position += velocity * global::dt;
  //   force *= 0.f;
  // }
};

