#pragma once

#include "Moveable.hpp"
#include "Shader.hpp"

class Camera : public Moveable {
public:
  static Camera* activeCam;

  Camera() = delete;
  Camera(Camera&) = delete;
  Camera(Camera&&) = delete;
  Camera(vec3 pos, float yaw = PI_2, float pitch = 0.f);

  static void setNextActiveCam();

  const float& getNearPlane()        const;
  const float& getFarPlane()         const;
  const float& getFov()              const;
  const float& getAspectRatio()      const;
  const mat4&  getProj()             const;
  const mat4&  getView()             const;
  const mat4&  getProjView()         const;
  const vec3&  getPositionRelative() const;

  mat4 getProjViewInv() const;
  mat4 getLocalView(vec3 pos) const;

  void setNearPlane(float p);
  void setFarPlane(float p);
  void setFlags(u32 f);
  void setUniforms(Shader& shader) const;
  void setPositionRelative(vec3 p);

  void update();

protected:
  friend struct gui;
  friend struct InputsHandler;

  int camIdx;

  float nearPlane = 0.1f;
  float farPlane = 100.f;
  float fov = 45.f;
  float aspectRatio = 1.f;

  mat4 proj = mat4(1.f);
  mat4 view = mat4(1.f);
  mat4 pv   = mat4(1.f);

  vec3 relativePos{};

private:
  static std::vector<Camera*> cameraPool;
  static size_t activeCamIdx;
};

