#include "TextureCubemap.hpp"

namespace gfx {

void TextureCubemap::initEmpty(ivec2 size, const TextureDescriptor& desc) {
  onInit(desc);
  for (int i = 0; i < 6; i++)
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, desc.internalFormat, size.x, size.y, 0, desc.format, desc.type, nullptr);
  unbind();
}

void TextureCubemap::loadFromImage(const fspath& path, const TextureDescriptor& desc) {
  loadFromImage(image2D(path), desc);
}

void TextureCubemap::loadFromImage(const image2D& img, const TextureDescriptor& desc) {
  onInit(desc);

  //     +Y
  //  -X +Z +X -Z
  //     -Y
  constexpr int skipCols[6] = {2, 0, 1, 1, 1, 3};
  constexpr int skipRows[6] = {1, 1, 0, 2, 1, 1};

  int faceSize = img.width / 4;
  glPixelStorei(GL_UNPACK_ROW_LENGTH, img.width);

  for (int i = 0; i < 6; i++) {
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, faceSize * skipCols[i]);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, faceSize * skipRows[i]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, desc.internalFormat, faceSize, faceSize, 0, desc.format, desc.type, img.pixels);
  }

  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  unbind();
}

void TextureCubemap::onInit(const TextureDescriptor& desc) {
  if (desc.target != GL_TEXTURE_CUBE_MAP)
    error("[TextureCubemap::TextureCubemap] Wrong target ({:#x})", desc.target);

  clear();
  target = desc.target;

  glGenTextures(1, &id);
  bind(0);
  glTexParameteri(desc.target, GL_TEXTURE_MIN_FILTER, desc.minFilter);
  glTexParameteri(desc.target, GL_TEXTURE_MAG_FILTER, desc.magFilter);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_S, desc.wrapS);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_T, desc.wrapT);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_R, desc.wrapR);
}

} // namespace gfx

