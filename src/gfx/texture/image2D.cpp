#include "image2D.hpp"

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "utils/utils.hpp"

namespace gfx {

void image2D::write(const std::string& path, uvec2 size, u8 channels, byte* buf) {
  stbi_flip_vertically_on_write(true);
  stbi_write_png(path.c_str(), size.x, size.y, channels, buf, size.x * channels);
}

image2D::image2D(image2D&& other) {
  width = other.width;
  height = other.height;
  channels = other.channels;
  pixels = other.pixels;
  path = other.path;
  flipVertically = other.flipVertically;
  loadType = other.loadType;

  other.pixels = nullptr;
  other.loadType = IMAGE2D_LOAD_NO;
}

image2D& image2D::operator=(image2D&& other) {
  if (this != &other) {
    clear();
    width = other.width;
    height = other.height;
    channels = other.channels;
    pixels = other.pixels;
    path = other.path;
    flipVertically = other.flipVertically;
    loadType = other.loadType;

    other.pixels = nullptr;
    other.loadType = IMAGE2D_LOAD_NO;
  }

  return *this;
}

image2D::image2D(int width, int height) : width(width), height(height) {}

image2D::image2D(fspath path, bool flipVertically, GLenum loadType) {
  load(path, flipVertically, loadType);
}

image2D::~image2D() {
  clear();
}

void image2D::load(fspath path, bool flipVertically, GLenum loadType) {
  clear();

  this->path = path;
  this->loadType = loadType;
  this->flipVertically = flipVertically;

  switch (loadType) {
    case IMAGE2D_LOAD_NO:
      error("[image2D::load] What?");
      break;
    case IMAGE2D_LOAD_STB:
      load_STB();
      break;
    case IMAGE2D_LOAD_STBF:
      load_STBF();
      break;
    default:
      error("[image2D::load] Unexpected loadType ({})", loadType);
  }

  if (!pixels)
    error(std::format("[image2D::load] Didn't load image [{}]", path.string()));
}

void image2D::load_STB() {
  stbi_set_flip_vertically_on_load(flipVertically);
  pixels = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
}

void image2D::load_STBF() {
  stbi_set_flip_vertically_on_load(flipVertically);
  pixels = stbi_loadf(path.string().c_str(), &width, &height, &channels, 0);
}

void image2D::clear() {
  switch (loadType) {
    case IMAGE2D_LOAD_NO:
      if (pixels)
        error("[image2D::clear] pixels is not nullptr");
      break;
    case IMAGE2D_LOAD_STB:
    case IMAGE2D_LOAD_STBF:
      stbi_image_free(pixels);
      break;
    default:
      error("[image2D::clear] Should never happen [{}]", loadType);
  }
  pixels = nullptr;
  loadType = IMAGE2D_LOAD_NO;
}

} // namespace gfx

