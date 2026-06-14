#pragma once

#include "../engine/mesh/Mesh.hpp"
#include "glm/common.hpp"
#include "glm/ext/matrix_transform.hpp"

struct Sun {
  float focus = 800.f;
  float intensity = 2.f;
  float yaw = 0.f;   // Radians
  float pitch = 0.f; // Radians
  float shadowSize = 100.f;
  float shadowDist = 1000.f;
  float shadowProjNear = 1.f;
  float shadowProjFar = 250.f;
  vec3 color{2.f, 1.8f, 1.4f}; // Is it okay?

  vec3 dir{-1.f, 0.f, 0.f}; // Towards light source
  mat4 lightSpace{1.f};

  void updateDir() {
    dir = normalize(vec3{
      cos(yaw) * cos(pitch),
      sin(pitch),
      sin(yaw) * cos(pitch)
    });
  }

  void updateLightSpace(const Camera* cam, ivec2 shadowResolution) {
    vec3 safeUp = vec3(0.f, 1.f, 0.f);

    mat4 lightProjection = glm::ortho(-shadowSize, shadowSize, -shadowSize, shadowSize, shadowProjNear, shadowProjFar);
    mat4 lightView = glm::lookAt(dir * shadowDist, vec3(0.f), safeUp);
    mat4 shadowMat = lightProjection * lightView;

    vec4 shadowOrigion = shadowMat * vec4(cam->getPosition(), 1.f);
    float texelsPerUnit = (float)shadowResolution.x / (shadowSize * 2.f);
    shadowOrigion = glm::round(shadowOrigion * texelsPerUnit) / texelsPerUnit;

    mat4 invShadowMat = glm::inverse(shadowMat);
    vec4 snappedWorldPos = invShadowMat * shadowOrigion;

    vec3 shadowCamPos = vec3(snappedWorldPos) + dir * shadowDist;
    lightView = glm::lookAt(shadowCamPos, vec3(snappedWorldPos), safeUp);

    lightSpace = lightProjection * lightView;
  }

  void setUniforms(Shader& shader) const {
    shader.setUniform1f("u_sunFocus", focus);
    shader.setUniform1f("u_sunIntensity", intensity);
    shader.setUniform3f("u_lightDir", dir);
    shader.setUniform3f("u_lightColor", color);
    shader.setUniformMatrix4f("u_lightSpace", lightSpace);
  }

  void draw(const Camera* cam, Shader& shader) {
    setUniforms(shader);
    shader.setUniformMatrix4f("u_camInv", cam->getProjViewInv());

    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);

    Mesh::drawScreen(cam, shader);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
  }
};

