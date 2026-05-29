#pragma once

#include "../engine/mesh/meshes.hpp"

struct Grid {
  float baseColor = 1.f;
  float gridSize = 20.f;
  MeshElements mesh = meshes::plane(2);

  void scale(float s) {
    mesh.scale(vec3{s, 1.f, s});
  }

  void draw(const Camera* camera, Shader& shader) {
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader.setUniform1f("u_baseColor", baseColor);
    shader.setUniform1f("u_gridSize", gridSize);

    mesh.setMatTranslation(camera->getPosition() * vec3(1.f, 0.f, 1.f));
    mesh.draw(camera, shader);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
  }
};

