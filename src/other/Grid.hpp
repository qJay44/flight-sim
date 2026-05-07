#pragma once

#include "../engine/mesh/meshes.hpp"

struct Grid : public Mesh {
  float baseColor = 1.f;
  float gridSize = 10.f;
  Mesh mesh = meshes::plane(2);

  void scale(float s) {
    mesh.scale(vec3{s, 1.f, s});
  }

  void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) {
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader.setUniform1f("u_baseColor", baseColor);
    shader.setUniform1f("u_gridSize", gridSize);

    mesh.setMatTranslation(camera->getPosition() * vec3(1.f, 0.f, 1.f));
    mesh.draw(camera, shader, forceNoWireframe);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
  }
};

