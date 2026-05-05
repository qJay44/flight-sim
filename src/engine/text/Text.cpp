#include "Text.hpp"

#include "glm/ext/matrix_clip_space.hpp"
#include "global.hpp"
#include "utils/utils.hpp"
#include "../mesh/Vertex.hpp"

Text::Text(const std::string& text) {
  setText(text);
}

vec2 Text::getPos() const {
  return matTranslation[3];
}

vec2 Text::getOrigin() const {
  return origin;
}

vec2 Text::getBorderSize() const {
  return borderSize;
}

void Text::setFont(Font* font) {
  this->font = font;
  generate();
}

void Text::setText(std::string text) {
  this->text = std::move(text);
  generate();
}

void Text::setColor(vec3 color) {
  this->color = color;
}

void Text::setPos(vec2 pos) {
  setMatTranslation({pos - origin, 0.f});
}

void Text::setScale(float scale) {
  setMatScale(scale);
}

void Text::setOrigin(vec2 o) {
  origin = o;
}

void Text::draw(const Camera* camera, Shader& shader) const {
  vec2 winSize = global::getWinSize();
  mat4 projection = glm::ortho(0.f, winSize.x, 0.f, winSize.y);
  projection *= getModel();

  for (const Glyph& g : glyphs) {
    shader.setUniform3f("u_color", color);
    g.tex->bind();

    g.draw(camera, shader, projection * g.getMatTranslation());
  }
}

void Text::generate() {
  if (!font)
    error("[Text::generate] Trying to generate text but no font provided");

  glyphs.clear();
  float cursorX = 0.f;

  for (char c : text) {
    const auto* chPtr = font->getCharacter(c);

    if (!chPtr) {
      warning("[Text::update] Didn't get char [{}] from font", c);
      continue;
    }

    const auto& ch = *chPtr;

    float x = ch.bearing.x;
    float y = -(ch.size.y - ch.bearing.y);
    float w = ch.size.x;
    float h = ch.size.y;

    std::vector<VertexPT> vertices =  {
      {{x    , y + h, 0.f}, {0.f, 0.f}},
      {{x    , y    , 0.f}, {0.f, 1.f}},
      {{x + w, y    , 0.f}, {1.f, 1.f}},

      {{x    , y + h, 0.f}, {0.f, 0.f}},
      {{x + w, y    , 0.f}, {1.f, 1.f}},
      {{x + w, y + h, 0.f}, {1.f, 0.f}},
    };

    Glyph glyph(vertices, GL_TRIANGLES, GL_STATIC_DRAW);
    glyph.tex = &ch.tex;
    glyph.offset = cursorX;
    glyph.translate({cursorX, 0.f, 0.f});

    glyphs.push_back(std::move(glyph));

    cursorX += ch.advance >> 6;
    borderSize.y = std::max(borderSize.y, h);
  }

  borderSize.x = cursorX;
}

