#include "Transformable.hpp"

#include "glm/gtc/quaternion.hpp"
#include "glm/ext/matrix_transform.hpp"

const mat4& Transformable::getMatTranslation() const { return matTranslation; }
const mat4& Transformable::getMatRotation()    const { return matRotation;   }
const mat4& Transformable::getMatScale()       const { return matScale; }

mat4 Transformable::getModel() const {
  return matTranslation * matRotation * matScale;
}

void Transformable::translate(const vec3& v) {
  matTranslation = glm::translate(matTranslation, v);
}

void Transformable::translate(const vec2& v) {
  matTranslation = glm::translate(matTranslation, vec3(v, 0.f));
}

void Transformable::translate(float v) {
  matTranslation = glm::translate(matTranslation, vec3(v));
}

void Transformable::rotate(float angle, const vec3& axis) {
  matRotation = glm::rotate(matRotation, angle, axis);
}

void Transformable::rotate(const glm::quat& q) {
  matRotation = glm::mat4_cast(q) * matRotation;
};

void Transformable::scale(const vec3& s)   { matScale = glm::scale(matScale, s); }
void Transformable::scale(float s)         { matScale = glm::scale(matScale, vec3(s)); }
void Transformable::scaleXY(const vec2& s) { matScale = glm::scale(matScale, vec3(s, 1.f)); }

void Transformable::setMatTranslation(const mat4& m)     { matTranslation = m; }
void Transformable::setMatTranslation(const vec3& pos)   { matTranslation = glm::translate(mat4(1.f), pos); }
void Transformable::setMatTranslationXY(const vec2& pos) { matTranslation = glm::translate(mat4(1.f), vec3(pos, 0.f)); }
void Transformable::setMatTranslation(float pos)         { matTranslation = glm::translate(mat4(1.f), vec3(pos, 0.f, 0.f)); }

void Transformable::setMatRotation(const mat4& m) { matRotation = m; }
void Transformable::setMatRotation(glm::quat q)   { matRotation = glm::mat4_cast(q); }

void Transformable::setMatScale(const mat4& m) { matScale = m; }
void Transformable::setMatScale(float s)       { matScale = glm::scale(mat4(1.f), vec3(s)); }
void Transformable::setMatScale(vec3 s)        { matScale = glm::scale(mat4(1.f), s); }
void Transformable::setMatScaleXZ(float s)     { matScale = glm::scale(mat4(1.f), vec3(s, 1.f, s)); }

