#include "Transformable.hpp"

#include "glm/gtc/quaternion.hpp"
#include "glm/ext/matrix_transform.hpp"

const mat4& Transformable::getTranslation() const { return transMat; }
const mat4& Transformable::getRotation()    const { return rotMat;   }
const mat4& Transformable::getScale()       const { return scaleMat; }

mat4 Transformable::getModel() const {
  return transMat * rotMat * scaleMat;
}

void Transformable::translate(const vec3& v) {
  transMat = glm::translate(transMat, v);
}

void Transformable::rotate(const float& angle, const vec3& axis) {
  rotMat = glm::rotate(rotMat, angle, axis);
}

void Transformable::rotate(const glm::quat& q) {
  rotMat = glm::mat4_cast(q) * rotMat;
};

void Transformable::scale(const float& s) { scaleMat = glm::scale(scaleMat, vec3(s)); }
void Transformable::scale(const vec2& s)  { scaleMat = glm::scale(scaleMat, vec3(s, 1.f)); }

void Transformable::setTranslationMat(const mat4& m) {
  transMat = m;
}

void Transformable::setTranslationMat(vec3 pos) {
  setTranslationMat(glm::translate(mat4(1.f), pos));
}

void Transformable::setRotationMat(const mat4& m) {
  rotMat = m;
}

void Transformable::setRotationMat(glm::quat q) {
  setRotationMat(glm::mat4_cast(q));
}

void Transformable::setScale(float s) {
  setScale(vec3(s));
}

void Transformable::setScale(vec3 s) {
  scaleMat = glm::scale(mat4(1.f), s);
}

