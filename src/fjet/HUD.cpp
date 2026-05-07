#include "HUD.hpp"

#include "glm/ext/quaternion_trigonometric.hpp"
#include "../engine/mesh/meshes.hpp"
#include "global.hpp"

HUD::HUD(Font* font) {
  TextureDescriptor desc;
  desc.uniformName = "u_tex";
  desc.internalFormat = GL_RGBA;
  desc.format = GL_RGBA;

  indicatorTex = Texture2D("res/tex/hud/indicator.png", desc);;

  vec2 winSize = global::getWinSize();
  vec2 winCenter = winSize * 0.5f;

  float arrowScaleScale = 0.5f; // Scaling initial proportions
  vec2 arrowOffset{0.3f, 0.f};
  vec2 arrowScale = vec2{0.085f, 0.05f} * arrowScaleScale;
  vec2 arrowSize = winSize * arrowScale;
  vec2 arrowTextOffset = arrowOffset * winCenter - vec2{arrowSize} * 0.25f;

  speedText.setFont(font);
  speedText.setText("Test");
  speedText.setOrigin(speedText.getBorderSize() * 0.5f);
  speedText.setColor(speedIndicator.color);
  speedText.setScale(arrowScaleScale);
  speedText.setPos(winCenter - arrowTextOffset);

  speedIndicator.mesh = meshes::rectangle();
  speedIndicator.mesh.translate(vec3(-arrowOffset, 0.f));
  speedIndicator.mesh.scale(arrowScale);
  speedIndicator.tex = &indicatorTex;

  altitudeText.setFont(font);
  altitudeText.setText("Test");
  altitudeText.setOrigin(altitudeText.getBorderSize() * 0.5f);
  altitudeText.setColor(speedIndicator.color);
  altitudeText.setScale(arrowScaleScale);
  altitudeText.setPos(winCenter + arrowTextOffset * vec2{1.f, -1.f} + vec2{arrowSize.x * 0.75f, 0.f}); // idk

  altitudeIndicator.mesh = meshes::rectangle();
  altitudeIndicator.mesh.translate(vec3(arrowOffset, 0.f));
  altitudeIndicator.mesh.rotate(glm::angleAxis(PI, vec3{0.f, 0.f, -1.f}));
  altitudeIndicator.mesh.scale(arrowScale);
  altitudeIndicator.tex = &indicatorTex;
}

void HUD::updateSpeed(float s) {
  int speedInt = static_cast<int>(s);
  speedText.setText(std::to_string(speedInt));
}

void HUD::updateAltitude(float a) {
  int altitudeInt = static_cast<int>(a);
  altitudeText.setText(std::to_string(altitudeInt));
}

void HUD::draw(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  speedIndicator.draw(camera, shader, forceNoWireframe);
  speedText.draw(camera, shader);
  altitudeIndicator.draw(camera, shader, forceNoWireframe);
  altitudeText.draw(camera, shader);

  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
}

void HUD::Indicator::draw(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  shader.setUniform3f("u_color", color);
  tex->bind();
  mesh.draw(camera, shader, forceNoWireframe);
}

