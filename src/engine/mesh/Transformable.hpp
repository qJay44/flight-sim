#pragma once

class Transformable {
public:
  const mat4& getMatTranslation() const;
  const mat4& getMatRotation()    const;
  const mat4& getMatScale()       const;
  mat4 getModel() const;

  void translate(const vec3& v);
  void translate(const vec2& v);
  void translate(float v);

  void rotate(float angle, const vec3& axis);
  void rotate(const glm::quat& q);

  void scale(const vec3& s);
  void scale(float s);
  void scaleXY(const vec2& s);

  void setMatTranslation(const mat4& m);
  void setMatTranslation(const vec3& pos);
  void setMatTranslationXY(const vec2& pos);
  void setMatTranslation(float pos);

  void setMatRotation(const mat4& m);
  void setMatRotation(glm::quat q);

  void setMatScale(const mat4& m);
  void setMatScale(float s);
  void setMatScale(vec3 s);
  void setMatScaleXZ(float s);

protected:
  mat4 matTranslation = mat4(1.f);
  mat4 matRotation    = mat4(1.f);
  mat4 matScale       = mat4(1.f);
};

