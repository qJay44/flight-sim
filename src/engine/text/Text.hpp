#pragma once

#include <cassert>

#include "../mesh/Mesh.hpp"
#include "../mesh/Transformable.hpp"
#include "../Camera.hpp"
#include "Font.hpp"

class Text : private Transformable {
public:
  Text() = default;

  Text(const std::string& text);

  vec2 getPos() const;
  vec2 getOrigin() const;
  vec2 getBorderSize() const;

  void setFont(Font* font);
  void setText(std::string text);
  void setColor(vec3 color);
  void setPos(vec2 pos);
  void setScale(float scale);
  void setOrigin(vec2 o);

  void draw(const Camera* camera, Shader& shader) const;

private:
  Font* font = nullptr;

  std::string text = "";
  vec3 color{1.f};
  vec2 borderSize{};
  vec2 origin{}; // Bottom-left

  struct Glyph : public Mesh {
    using Mesh::Mesh;
    const Texture2D* tex;
    float offset;
  };

  std::vector<Glyph> glyphs;

private:
  void generate();
};

