#pragma once

#include "../../Camera.hpp"
#include "Plane.hpp"

namespace core::math::frustum {

struct Frustum {
  Plane topFace;
  Plane bottomFace;

  Plane rightFace;
  Plane leftFace;

  Plane farFace;
  Plane nearFace;

  Frustum(const Camera& cam) {
    // TODO: Is this correct?
    vec3 camLeft = normalize(cross(cam.orientation, cam.up));
    vec3 camRight = -camLeft;
    vec3 camBack = -cam.orientation;

    float aspectRatio = cam.cachedProj[1][1] / cam.cachedProj[0][0];
    float halfVSide = cam.farPlane * tanf(cam.fov * 0.5f);
    float halfHSide = halfVSide * aspectRatio;
    vec3 frontMultFar = cam.farPlane * cam.orientation;

    // Pointing inside the frustum
    vec3 right = normalize(cross(frontMultFar + camLeft  * halfHSide,  cam.up));
    vec3 left  = normalize(cross(frontMultFar + camRight * halfHSide, -cam.up));
    vec3 up    = normalize(cross(frontMultFar + cam.up     * halfVSide,  camRight));
    vec3 down  = normalize(cross(frontMultFar - cam.up     * halfVSide,  camLeft));

    farFace    = {camBack, dot(camBack, cam.position + frontMultFar)};
    nearFace   = {cam.orientation, dot(cam.orientation, cam.position + cam.nearPlane * cam.orientation)};
    rightFace  = {right, dot(right, cam.position)};
    leftFace   = {left , dot(left, cam.position)};
    topFace    = {up   , dot(up, cam.position)};
    bottomFace = {down , dot(down, cam.position)};
  }
};

} // namespace core::math::frustum

