 #pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../texture/Texture2D.hpp"
#include "utils/utils.hpp"

class Font {
public:
  struct Character {
    Texture2D tex;
    ivec2 size;
    ivec2 bearing;
    long advance;
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

  const Character* getCharacter(char c) const {
    auto it = characters.find(c);
    if (it == characters.end())
      return nullptr;

    return &it->second;
  }

private:
  FT_Library ft;
  FT_Face face;

  std::map<char, Character> characters;

private:
  void loadCharacters() {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (u32 c = 0; c < 128; c++) {
      if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        warning("[Font::loadCharacters] Failed to load glyph [{}]", c);
        continue;
      }

      ivec2 size{face->glyph->bitmap.width, face->glyph->bitmap.rows};
      image2D img;
      img.width = size.x;
      img.height = size.y;
      img.pixels = face->glyph->bitmap.buffer;

      Texture2D tex = Texture2D(img, {
        .uniformName = "u_tex",
        .internalFormat = GL_RED,
        .format = GL_RED
      });

      Character character = {
        std::move(tex),
        size,
        {face->glyph->bitmap_left, face->glyph->bitmap_top},
        face->glyph->advance.x
      };

      characters.emplace(c, std::move(character));

      img.pixels = nullptr;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }
};

