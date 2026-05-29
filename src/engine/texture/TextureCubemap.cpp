#include "TextureCubemap.hpp"

void TextureCubemap::onInit(const TextureDescriptor& desc) {
  if (desc.target != GL_TEXTURE_CUBE_MAP)
    error("[TextureCubemap::TextureCubemap] Wrong target ({:#x})", desc.target);

  target = desc.target;

  glGenTextures(1, &id);
  bind(0);
  glTexParameteri(desc.target, GL_TEXTURE_MIN_FILTER, desc.minFilter);
  glTexParameteri(desc.target, GL_TEXTURE_MAG_FILTER, desc.magFilter);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_S, desc.wrapS);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_T, desc.wrapT);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_R, desc.wrapR);
}

TextureCubemap TextureCubemap::loadFromImage(const fspath& path, const TextureDescriptor& desc) {
  return loadFromImage(image2D(path), desc);
}

TextureCubemap TextureCubemap::loadFromImage(const image2D& img, const TextureDescriptor& desc) {
  TextureCubemap tex;

  tex.onInit(desc);
  tex.bind(0);

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

  if (desc.genMipMap)
    glGenerateMipmap(desc.target);

  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  tex.unbind();

  return tex;
}

