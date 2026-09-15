#pragma once

#include "glm/ext/matrix_transform.hpp"
#include "../components/TransformComponent.hpp"
#include "glm/gtc/quaternion.hpp"

namespace ecs::TransformSystem {

inline mat4 getModel(const component::TransformComponent& trans) {
  mat4 t = glm::translate(mat4(1.f), trans.pos);
  mat4 r = glm::mat4_cast(trans.rotation);
  mat4 s = glm::scale(mat4(1.f), trans.scale);

  return t * r * s;
}

};

