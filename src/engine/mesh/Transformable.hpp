#pragma once

class Transformable {
public:
  const mat4& getTranslation() const;
  const mat4& getRotation()    const;
  const mat4& getScale()       const;
  mat4 getModel() const;

  void translate(const vec3& v);
  void rotate(const float& angle, const vec3& axis);
  void rotate(const glm::quat& q);
  void scale(const float& s);
  void scale(const vec2& s);

  void setTrans(const mat4& t);
  void setTrans(vec3 pos);

  void setScale(float s);
  void setScale(vec3 s);

protected:
  mat4 transMat = mat4(1.f);
  mat4 rotMat   = mat4(1.f);
  mat4 scaleMat = mat4(1.f);
};

