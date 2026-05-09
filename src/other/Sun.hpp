#pragma once

#include "../engine/mesh/Mesh.hpp"

struct Sun {
  float focus = 800.f;
  float intensity = 2.f;
  float yaw = PI_2;
  float pitch = 0.f;
  vec3 color{1.f};
  vec3 skyHorizonColor = vec3(1.f);
  vec3 skyZenithColor{0.289f, 0.565f, 1.f};
  vec3 groundColor = vec3(0.637f);

  vec3 dir{-1.f, 0.f, 0.f}; // From sun

  void updateDir() {
    dir = normalize(vec3{
      cos(yaw) * cos(pitch),
      sin(pitch),
      sin(yaw) * cos(pitch)
    });
  }

  void setUniforms(Shader& shader) {
    shader.setUniform1f("u_sunFocus", focus);
    shader.setUniform1f("u_sunIntensity", intensity);
    shader.setUniform3f("u_sunDir", dir);
    shader.setUniform3f("u_sunColor", color);
  }

  void setUniformsEnvironment(Shader& shader) {
    shader.setUniform3f("u_skyHorizonColor", skyHorizonColor);
    shader.setUniform3f("u_skyZenithColor", skyZenithColor);
    shader.setUniform3f("u_groundColor", groundColor);
  }

  void draw(const Camera* cam, Shader& shader) {
    setUniforms(shader);
    shader.setUniformMatrix4f("u_camInv", cam->getProjViewInv());

    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);

    Mesh::screenDraw(cam, shader);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
  }
};

