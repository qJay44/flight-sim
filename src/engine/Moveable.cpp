#include "Moveable.hpp"

#include "glm/gtx/vector_angle.hpp"
#include "global.hpp"

Moveable::Moveable(vec3 pos, float yaw, float pitch) : position(pos), yaw(yaw), pitch(pitch) {
  calcOrientation();
}

void Moveable::accelerate(bool b) {
  speed = speedDefault * !b + speedDefault * speedMul * b;
}

const float& Moveable::getSpeedDefault()    const { return speedDefault; }
const float& Moveable::getSpeedMultiplier() const { return speedMul;     }
const float& Moveable::getSensitivity()     const { return sensitivity;  }
const float& Moveable::getYaw()             const { return yaw;          }
const float& Moveable::getPitch()           const { return pitch;        }
const vec3&  Moveable::getOrientation()     const { return orientation;  }
const vec3&  Moveable::getPosition()        const { return position;     }
const vec3&  Moveable::getUp()              const { return up;           }

vec3 Moveable::getBack()    const { return -getForward();};
vec3 Moveable::getLeft()    const { return -getRight(); }
vec3 Moveable::getRight()   const { return normalize(cross(up, -orientation));};
vec3 Moveable::getForward() const { return getOrientation();};
vec3 Moveable::getDown()    const { return -getUp();};

void Moveable::setSpeedDefault(float n)      { speedDefault = n;   }
void Moveable::setSpeedMultiplier(float n)   { speedMul     = n;   }
void Moveable::setSensitivity(float n)       { sensitivity  = n;   }
void Moveable::setYaw(float n)               { yaw          = n;   }
void Moveable::setPitch(float n)             { pitch        = n;   }
void Moveable::setOrientation(const vec3& o) { orientation  = o;   }
void Moveable::setPosition(const vec3& pos)  { position     = pos; }
void Moveable::setUp(const vec3& up)         { this->up     = up;  }

void Moveable::setView(const Moveable* rhs) {
  up = rhs->up;
  yaw = rhs->yaw;
  pitch = rhs->pitch;
  orientation = rhs->orientation;
}

void Moveable::moveForward() { position +=  orientation * speed * global::dt; }
void Moveable::moveBack()    { position += -orientation * speed * global::dt; }
void Moveable::moveLeft()    { position += getLeft()  * speed * global::dt; }
void Moveable::moveRight()   { position += getRight() * speed * global::dt; }
void Moveable::moveUp()      { position +=  up * speed * global::dt; }
void Moveable::moveDown()    { position += -up * speed * global::dt; }

void Moveable::onMouseMove(dvec2 mousePos) {
  if (global::guiFocused)
    return;

  dvec2 winSize = global::getWinSize();
  dvec2 winCenter = winSize * 0.5;
  dvec2 distFromCenter = mousePos - winCenter;

  dvec2 delta = dvec2(sensitivity) * distFromCenter / winCenter;
  yaw += delta.x;
  pitch = std::clamp(pitch - delta.y, -PI_2 + 0.1, PI_2 - 0.1);

  calcOrientation();
}

void Moveable::onMouseScroll(dvec2 offset) {
  warning("[Moveable::onMouseScroll] Nothing to do...");
}

void Moveable::calcOrientation() {
  orientation = normalize(vec3{
    cos(yaw) * cos(pitch),
    sin(pitch),
    sin(yaw) * cos(pitch)
  });
}

