#include "HUD.hpp"

#include "glm/ext/quaternion_trigonometric.hpp"
#include "global.hpp"

HUD::HUD(Font* font, Shader* shaderText) : shaderText(shaderText) {
  TextureDescriptor desc{};
  desc.internalFormat = GL_RGBA;
  desc.format = GL_RGBA;

  indicatorTex = Texture2D("res/tex/hud/indicator.png", desc);

  const vec2 winSize = global::getWinSize();
  const vec2 winCenter = winSize * 0.5f;
  const vec2 imgScale{65.f, 25.f};
  const vec2 imgOffsetFromCenter = {winSize.x * 0.15f, 0.f};

  speedText.setFont(font);
  speedText.setColor({0.f, 1.f, 0.f});

  altitudeText.setFont(font);
  altitudeText.setColor({0.f, 1.f, 0.f});
  altitudeText.setPos(winCenter);
  altitudeText.setPos(imgOffsetFromCenter);

  speedImg.translate(winCenter);
  speedImg.translate(-imgOffsetFromCenter);
  speedImg.scale(imgScale);

  altitudeImg.translate(winCenter);
  altitudeImg.translate(imgOffsetFromCenter);
  altitudeImg.scale(imgScale);
  altitudeImg.rotate(glm::angleAxis(PI, vec3{0.f, 0.f, -1.f}));
}

void HUD::updateSpeed(float s) {
  int speedInt = static_cast<int>(s);
  const auto speedStr = std::to_string(speedInt);
  speedText.setText(speedStr);

  const vec2 winSize = global::getWinSize();
  const vec2 winCenter = winSize * 0.5f;
  const vec2 imgOffsetFromCenter = {winSize.x * 0.15f, 0.f};
  const vec2 imgScale{65.f, 25.f};
  const vec2 txtOffsetL = imgOffsetFromCenter + vec2{imgScale.x * 0.55f, 0.f};

  vec2 txtOffsetR = txtOffsetL;
  txtOffsetR.x -= altitudeText.getRectSize().x / speedStr.size();

  speedText.setOriginCenter();
  speedText.setPos(winCenter - txtOffsetL);

  altitudeText.setOriginCenter();
  altitudeText.setPos(winCenter + txtOffsetR);
}

void HUD::updateAltitude(float a) {
  int altitudeInt = static_cast<int>(a);
  altitudeText.setText(std::to_string(altitudeInt));
}

void HUD::draw(const Camera* camera, Shader& shader) const {
  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  shader.setUniform3f("u_color", {0.f, 1.f, 0.f});
  shader.setUniformMatrix4f("u_proj", global::getScreenProjection());

  indicatorTex.bind();
  speedImg.draw(camera, shader);
  altitudeImg.draw(camera, shader);

  speedText.draw(camera, *shaderText);
  altitudeText.draw(camera, *shaderText);

  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
}

