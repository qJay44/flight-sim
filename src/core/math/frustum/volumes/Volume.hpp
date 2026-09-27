#pragma once

#include "../Frustum.hpp"
#include "../Plane.hpp"

namespace core::math::frustum::volume {

struct Volume {
  virtual bool isOnOrForwardPlane(const Plane& plane) const = 0;
  virtual bool isOnFrustum(const Frustum& frustum) const = 0;
  virtual bool isOnFrustum(const Frustum& frustum, const mat4& matTranslate, const mat4& matRotate, const mat4& matScale) const = 0;
};

} // namespace core::frustum::volume

