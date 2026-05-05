#pragma once

#include "../engine/mesh/meshes.hpp"
#include "../engine/texture/Texture2D.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"

class HUD {
public:
  HUD() {
    TextureDescriptor desc;
    desc.uniformName = "u_tex";
    desc.internalFormat = GL_RGBA;
    desc.format = GL_RGBA;

    indicatorTex = Texture2D("res/tex/hud/indicator.png", desc); desc.unit++;

    vec3 arrowOffset = {0.3f, 0.f, 0.f};
    float arrowScale = 0.05f;

    speed.mesh = meshes::plane();
    speed.mesh.scale(arrowScale);
    speed.mesh.translate(-arrowOffset);
    speed.tex = &indicatorTex;

    altitude.mesh = meshes::plane();
    altitude.mesh.translate(arrowOffset);
    altitude.mesh.rotate(glm::angleAxis(PI, vec3{0.f, 1.f, 0.f}));
    altitude.mesh.scale(arrowScale);
    altitude.tex = &indicatorTex;
  }

  void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) {
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    speed.draw(camera, shader, forceNoWireframe);
    altitude.draw(camera, shader, forceNoWireframe);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
  }

private:
  struct Indicator {
    Mesh mesh;
    Texture2D* tex;
    vec3 color{0.f, 1.f, 0.f};

    void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) {
      shader.setUniform3f("u_color", color);
      tex->bind();
      mesh.draw(camera, shader, forceNoWireframe);
    }
  };

  Texture2D indicatorTex;

  Indicator speed;
  Indicator altitude;
};

