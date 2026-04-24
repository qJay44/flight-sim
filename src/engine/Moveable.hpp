#pragma once

class Moveable {
public:
  Moveable(vec3 pos = vec3(0.f), float yaw = PI_2, float pitch = 0.f);

  void accelerate(bool b);

  const float& getSpeedDefault()    const;
  const float& getSpeedMultiplier() const;
  const float& getSensitivity()     const;
  const float& getSpeed()           const;
  const float& getYaw()             const;
  const float& getPitch()           const;
  const vec3&  getOrientation()     const;
  const vec3&  getPosition()        const;
  const vec3&  getUp()              const;

  vec3 getBack()    const;
  vec3 getLeft()    const;
  vec3 getRight()   const;
  vec3 getForward() const;
  vec3 getDown()    const;

  void setSpeedDefault(float n);
  void setSpeedMultiplier(float n);
  void setSensitivity(float n);
  void setYaw(float n);
  void setPitch(float n);
  void setOrientation(const vec3& o);
  void setPosition(const vec3& pos);
  void setUp(const vec3& up);
  void setView(const Moveable* rhs);

  virtual void moveForward();
  virtual void moveBack();
  virtual void moveLeft();
  virtual void moveRight();
  virtual void moveUp();
  virtual void moveDown();
  virtual void onMouseMove(dvec2 mousePos);
  virtual void onMouseScroll(dvec2 offset);

protected:
  vec3 position{};
  float yaw = -PI;
  float pitch = 0.f;

  vec3 orientation{};
  vec3 up{0.f, 1.f, 0.f};
  float sensitivity = 1.f;
  float speedDefault = 1.f;
  float speedMul = 5.f;
  float speed = speedDefault * speedMul;

protected:
  virtual void calcOrientation();
};

