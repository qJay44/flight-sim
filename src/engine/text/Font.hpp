 #pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../texture/Texture2D.hpp"
#include "utils/utils.hpp"

class Font {
public:
  struct Glyph {
    vec2 uv0, uv1;
    u32 width, height;
    FT_Int bearingX, bearingY;
    FT_Pos advance;
  };

  Font(const Font&) = delete;
  Font& operator=(const Font&) = delete;

  Font(Font&&) = default;
  Font& operator=(Font&&) = default;

  Font(const fspath& path, u32 height, u32 width = 0) {
    if (FT_Init_FreeType(&ft))
      error("[Font::Font] Could not ini FreeType Library");

    if (FT_New_Face(ft, path.string().c_str(), 0, &face))
      error("[Font::Font] Failed to load font [{}]", path.string());

    FT_Set_Pixel_Sizes(face, width, height);

    loadCharacters();
  }

  ~Font() {
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
  }

  const Font::Glyph& getGlyph(char c) {
    return glyphs[(int)c];
  }

  void bindAtlas(GLuint unit) const {
    atlas.bind(unit);
  }

private:

  FT_Library ft;
  FT_Face face;
  Glyph glyphs[128];
  Texture2D atlas;

private:
  void loadCharacters() {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    TextureDescriptor texDesc{};
    texDesc.internalFormat = GL_RED;
    texDesc.format = GL_RED;

    u32 atlasSize = 512;
    float atlasSizeInv = 1.f / (float)atlasSize;

    atlas = Texture2D(ivec2(atlasSize), texDesc);
    int atlasX = 0;
    int atlasY = 0;
    u32 maxRowHeight = 0;
    atlas.bind();

    for (u8 c = 0; c < 128; c++) {
      if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        warning("[Font::loadCharacters] Failed to load glyph [{}]", c);
        continue;
      }

      FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF);

      FT_Bitmap bmp = face->glyph->bitmap;
      Glyph& glyph = glyphs[c];

      if (atlasX + bmp.width >= atlasSize) {
        atlasX = 0;
        atlasY += maxRowHeight + 1;
        maxRowHeight = 0;
      }

      if (bmp.width > 0 && bmp.rows > 0)
        glTexSubImage2D(GL_TEXTURE_2D, 0, atlasX, atlasY, bmp.width, bmp.rows, GL_RED, GL_UNSIGNED_BYTE, bmp.buffer);

      glyph.uv0.x = (float)atlasX * atlasSizeInv;
      glyph.uv0.y = (float)atlasY * atlasSizeInv;
      glyph.uv1.x = (float)(atlasX + bmp.width) * atlasSizeInv;
      glyph.uv1.y = (float)(atlasY + bmp.rows) * atlasSizeInv;

      glyph.width = bmp.width;
      glyph.height = bmp.rows;
      glyph.bearingX = face->glyph->bitmap_left;
      glyph.bearingY = face->glyph->bitmap_top;
      glyph.advance = face->glyph->advance.x >> 6;

      glyphs[c] = glyph;

      atlasX += bmp.width + 1;
      if (bmp.rows > maxRowHeight)
        maxRowHeight = bmp.rows;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }
};

