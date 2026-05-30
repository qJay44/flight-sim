#pragma once

#include "../engine/mesh/Mesh.hpp"

struct Grid {
  float baseColor = 1.f;
  float gridSize = 20.f;

  void draw(const Camera* camera, Shader& shader) {
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vec3 relativePos = vec3(0.f, camera->getPosition().y, 0.f);
    mat4 localView = camera->getLocalView(relativePos);

    shader.setUniformMatrix4f("u_localView", localView);
    Mesh::drawScreen(camera, shader);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
  }
};

