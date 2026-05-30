#include "Camera.hpp"

#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/matrix.hpp"
#include "glm/trigonometric.hpp"

#include "global.hpp"

std::vector<Camera*> Camera::cameraPool;
Camera* Camera::activeCam = nullptr;
size_t Camera::activeCamIdx = 0;

Camera::Camera(vec3 pos, float yaw, float pitch) : Moveable(pos, yaw, pitch) {
  update();
  cameraPool.push_back(this);
  camIdx = cameraPool.size() - 1;
  activeCam = activeCam ?: this;
};

void Camera::setNextActiveCam() {
  activeCamIdx = (activeCamIdx + 1) % cameraPool.size();
  activeCam = cameraPool[activeCamIdx];
}

const float& Camera::getNearPlane()        const { return nearPlane;   }
const float& Camera::getFarPlane()         const { return farPlane;    }
const float& Camera::getFov()              const { return fov;         }
const float& Camera::getAspectRatio()      const { return aspectRatio; }
const mat4&  Camera::getProj()             const { return proj;        }
const mat4&  Camera::getView()             const { return view;        }
const mat4&  Camera::getProjView()         const { return pv;          }
const vec3&  Camera::getPositionRelative() const { return relativePos; }

mat4 Camera::getProjViewInv() const {
  return glm::inverse(pv);
}

mat4 Camera::getLocalView(vec3 pos) const {
  return glm::lookAt(pos, pos + orientation, up);
}

void Camera::setNearPlane(float p) { nearPlane = p; }
void Camera::setFarPlane(float p) { farPlane = p; }

void Camera::setUniforms(Shader& shader) const {
  shader.setUniform1f      ("u_camNear"   , getNearPlane());
  shader.setUniform1f      ("u_camFar"    , getFarPlane());
  shader.setUniform1f      ("u_camFov"    , getFov());
  shader.setUniform3f      ("u_camPos"    , getPosition());
  shader.setUniform3f      ("u_camRight"  , getRight());
  shader.setUniform3f      ("u_camUp"     , getUp());
  shader.setUniform3f      ("u_camForward", getForward());
  shader.setUniformMatrix4f("u_camProj"   , getProj());
  shader.setUniformMatrix4f("u_camView"   , getView());
  shader.setUniformMatrix4f("u_camPV"     , getProjView());
  shader.setUniformMatrix4f("u_camInvPV"  , getProjViewInv());
}

void Camera::setPositionRelative(vec3 p) {
  relativePos = p;
}

void Camera::update() {
  vec2 winSize = global::getWinSize();

  aspectRatio = winSize.x / winSize.y;
  proj = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
  view = glm::lookAt(position, position + orientation, up);
  pv = proj * view;
}

