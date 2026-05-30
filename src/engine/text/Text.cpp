#include "Text.hpp"

#include <string_view>

#include "global.hpp"
#include "../mesh/MeshData.hpp"

Text::Text() {
  MeshData data{};
  data.verticesSize = sizeof(vertex::PT) * TEXT_MAX_LEN * 6;
  data.layout = vertex::PT_LAYOUT;
  data.usage = GL_DYNAMIC_DRAW;

  mesh = MeshArrays(data);
}

Text::Text(Font* font, const std::string& text = "") : Text() {
  this->font = font;

  setText(text);
}

vec2 Text::getPos() const {
  return mesh.getMatTranslation()[3];
}

vec2 Text::getOrigin() const {
  return origin;
}

vec2 Text::getRectSize() const {
  return rectSize;;
}

void Text::setFont(Font* font) {
  this->font = font;
}

void Text::setText(std::string text) {
  generate(text);
}

void Text::setColor(vec3 color) {
  this->color = color;
}

void Text::setColorOutline(vec3 color) {
  this->colorOutline = color;
}

void Text::setPos(vec2 pos) {
  mesh.setMatTranslation(pos - origin);
}

void Text::setOrigin(vec2 o) {
  origin = o;
}

void Text::setOriginCenter() {
  origin = rectSize * 0.25f; // idk
}

void Text::setOutlineThreshold(float t) {
  outlineThreshold = t;
}

void Text::draw(const Camera* camera, Shader& shader) const {
  shader.setUniformMatrix4f("u_proj", global::getScreenProjection());
  shader.setUniform3f("u_color", color);
  shader.setUniform3f("u_colorOutline", colorOutline);
  shader.setUniform1f("u_outlineThreshold", outlineThreshold);

  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  font->bindAtlas(0);
  mesh.draw(camera, shader);

  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
}

void Text::generate(std::string_view text) {
  const size_t textLen = text.length();

  if (textLen > TEXT_MAX_LEN) {
    fprintf(stderr, "[Text::generate] Text length [%zu] exceeds limit [%u]\n", textLen, TEXT_MAX_LEN);
    exit(EXIT_FAILURE);
  }

  float cursorX = 0.f;

  MeshData meshData;
  vertex::PT vertices[TEXT_MAX_LEN * 6];
  size_t verticesCount = 0;

  for (size_t i = 0; i < textLen; i++) {
    if (text[i] == '\0')
      break;

    const auto& g = font->getGlyph(text[i]);
    size_t triIdx = i * 6;

    float x = cursorX + g.bearingX;
    float y = -(float)g.height + g.bearingY;
    float w = g.width;
    float h = g.height;

    vertices[triIdx + 0] = {{x    , y + h, 0.f}, {g.uv0.x, g.uv0.y}};
    vertices[triIdx + 1] = {{x    , y    , 0.f}, {g.uv0.x, g.uv1.y}};
    vertices[triIdx + 2] = {{x + w, y    , 0.f}, {g.uv1.x, g.uv1.y}};

    vertices[triIdx + 3] = {{x    , y + h, 0.f}, {g.uv0.x, g.uv0.y}};
    vertices[triIdx + 4] = {{x + w, y    , 0.f}, {g.uv1.x, g.uv1.y}};
    vertices[triIdx + 5] = {{x + w, y + h, 0.f}, {g.uv1.x, g.uv0.y}};

    cursorX += g.advance;
    rectSize.y = fmaxf(rectSize.y, h);
    verticesCount += 6;
  }

  meshData.vertices = (float*)vertices;
  meshData.verticesSize = sizeof(vertices[0]) * verticesCount;
  meshData.layout = vertex::PT_LAYOUT;

  rectSize.x = cursorX;
  mesh.updateBufferVBO(meshData);
}

