#include "Texture2D.hpp"

Texture2D Texture2D::debug0Tex;

const Texture2D& Texture2D::getDebug0Tex() {
  if (debug0Tex.desc.uniformName.empty())
    debug0Tex = Texture2D(
      {"res/tex/debug/uvChecker.jpg", IMAGE2D_LOAD_STB , true},
      {
        .uniformName = "u_debug0Tex",
        .unit = 0,
        .minFilter = GL_NEAREST,
        .magFilter = GL_NEAREST,
        .wrapS = GL_REPEAT,
        .wrapT = GL_REPEAT,
    });

  return debug0Tex;
}

Texture2D::Texture2D(const image2D& img, const TextureDescriptor& d) : Texture(d) {
  if (desc.target != GL_TEXTURE_2D)
    error("[Texture2D::Texture2D] Wrong tartget for [{}]", desc.uniformName);

  glGenTextures(1, &id);
  bind();
  glTexParameteri(desc.target, GL_TEXTURE_MIN_FILTER, desc.minFilter);
  glTexParameteri(desc.target, GL_TEXTURE_MAG_FILTER, desc.magFilter);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_S, desc.wrapS);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_T, desc.wrapT);
  glTexImage2D(desc.target, 0, desc.internalFormat, img.width, img.height, 0, desc.format, desc.type, img.pixels);

  if (desc.genMipMap)
    glGenerateMipmap(desc.target);

  unbind();
}

Texture2D::Texture2D(const ivec2& size, const TextureDescriptor& desc)
  : Texture2D(image2D{size.x, size.y}, desc) {}

Texture2D::Texture2D(const fspath& path, const TextureDescriptor& desc)
  : Texture2D(image2D(path), desc) {}

void Texture2D::upload(ivec2 coord, ivec2 size, const void* data, GLenum type) const {
  bind();
  glTexSubImage2D(desc.target, 0, coord.x, coord.y, size.x, size.y, desc.format, type, data);
  unbind();
}

