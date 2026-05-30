#pragma once

#include <cassert>
#include <string_view>

#include "../mesh/MeshArrays.hpp"
#include "../Camera.hpp"
#include "Font.hpp"

#define TEXT_MAX_LEN 256u

class Text {
public:
  Text();
  Text(Font* font, const std::string& text);

  vec2 getPos() const;
  vec2 getOrigin() const;
  vec2 getRectSize() const;

  void setFont(Font* font);
  void setText(std::string text);
  void setOrigin(vec2 o);
  void setOriginCenter();
  void setPos(vec2 pos);
  void setColor(vec3 color);
  void setColorOutline(vec3 color);
  void setOutlineThreshold(float t);

  void draw(const Camera* camera, Shader& shader) const;

  float maxHeight = FLT_MIN;
private:
  Font* font = nullptr;

  vec2 origin{}; // Bottom-left initially
  vec2 rectSize{};
  vec3 color{1.f};
  vec3 colorOutline{0.f};
  float outlineThreshold = 0.35f; // basically a threshold where outline starts, decrease to make thicker

  MeshArrays mesh;

private:
  void generate(std::string_view text);
};

