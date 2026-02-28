#include "Camera.hpp"

#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/matrix.hpp"
#include "glm/trigonometric.hpp"

#include "mesh/meshes.hpp"
#include "global.hpp"

Camera::Camera(vec3 pos, float yaw, float pitch) : Moveable(pos, yaw, pitch) {
  update();
};

const float& Camera::getNearPlane()   const { return nearPlane;   }
const float& Camera::getFarPlane()    const { return farPlane;    }
const float& Camera::getFov()         const { return fov;         }
const float& Camera::getAspectRatio() const { return aspectRatio; }
const mat4&  Camera::getProj()        const { return proj;        }
const mat4&  Camera::getView()        const { return view;        }
const mat4&  Camera::getProjView()    const { return pv;          }

mat4 Camera::getProjViewInv() const {
  return glm::inverse(pv);
}

void Camera::setNearPlane(float p) { nearPlane = p; }
void Camera::setFarPlane(float p) { farPlane = p; }
void Camera::setFlags(u32 f) { flags = f; }

void Camera::update() {
  vec2 winSize = global::getWinSize();

  aspectRatio = winSize.x / winSize.y;
  proj = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
  view = glm::lookAt(position, position + orientation, up);
  pv = proj * view;

  dvec2 winCenter = global::getWinCenter();
  if (!global::guiFocused)
    glfwSetCursorPos(global::window, winCenter.x, winCenter.y);
}

void Camera::draw(const Camera* cam, Shader& shader) const {
  if (this != cam) {
    const vec3& p = position;

    if (flags & CameraFlags_DrawRight)
      meshes::line(p, p + getRight(), global::red).draw(cam, shader);

    if (flags & CameraFlags_DrawUp)
      meshes::line(p, p + up, global::green).draw(cam, shader);

    if (flags & CameraFlags_DrawForward)
      meshes::line(p, p + getForward(), global::blue).draw(cam, shader);
  }
}

