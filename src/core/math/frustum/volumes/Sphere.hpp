#pragma once

#include "Volume.hpp"

namespace core::math::frustum::volume {

struct Sphere : public Volume {
  vec3 center{0.f};
  float radius = 0.f;

  Sphere(vec3 center, float radius) : center(center), radius(radius) {}

  bool isOnOrForwardPlane(const Plane& plane) const final {
    return plane.getSignedDistanceToPlane(center) > -radius;
  }

  bool isOnFrustum(const Frustum& frustum) const final {
    Sphere globalSphere(center, radius);

    return (
      globalSphere.isOnOrForwardPlane(frustum.leftFace)  &&
      globalSphere.isOnOrForwardPlane(frustum.rightFace) &&
      globalSphere.isOnOrForwardPlane(frustum.farFace)   &&
      globalSphere.isOnOrForwardPlane(frustum.nearFace)  &&
      globalSphere.isOnOrForwardPlane(frustum.topFace)   &&
      globalSphere.isOnOrForwardPlane(frustum.bottomFace)
    );
  }

  bool isOnFrustum(const Frustum& frustum, const mat4& matTranslate, const mat4& matRotate, const mat4& matScale) const final {
    mat4 model = matTranslate * matRotate * matScale;
    vec3 globalScale{matScale[0][0], matScale[1][1], matScale[2][2]};
    vec3 globalCenter = model * vec4(center, 1.f);
    float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);

    Sphere globalSphere(globalCenter, radius * (maxScale * 0.5f));

    return (
      globalSphere.isOnOrForwardPlane(frustum.leftFace)  &&
      globalSphere.isOnOrForwardPlane(frustum.rightFace) &&
      globalSphere.isOnOrForwardPlane(frustum.farFace)   &&
      globalSphere.isOnOrForwardPlane(frustum.nearFace)  &&
      globalSphere.isOnOrForwardPlane(frustum.topFace)   &&
      globalSphere.isOnOrForwardPlane(frustum.bottomFace)
    );
  }
};

} // namespace core::math::frustum::volume

