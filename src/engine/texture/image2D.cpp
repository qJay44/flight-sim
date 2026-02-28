#include "image2D.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "utils/loadTif.hpp"

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

image2D::image2D(fspath path, GLenum loadType, bool flipVertically) {
  load(path, loadType, flipVertically);
}

image2D::~image2D() {
  clear();
}

void image2D::load(fspath path, GLenum loadType, bool flipVertically) {
  clear();

  this->path = path;
  this->loadType = loadType;
  this->flipVertically = flipVertically;

  switch (loadType) {
    case IMAGE2D_LOAD_NO:
      error("[image2D::load] What?");
      break;
    case IMAGE2D_LOAD_R16I:
      loadTif_R16I();
      break;
    case IMAGE2D_LOAD_R16UI:
      loadTif_R16UI();
      break;
    default:
      load_STB();
      this->loadType = IMAGE2D_LOAD_STB;
  }

  if (!pixels)
    error(std::format("[image2D::load] Didn't load image [{}]", path.string()));
}

void image2D::load_STB() {
  stbi_set_flip_vertically_on_load(flipVertically);
  pixels = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
}

void image2D::loadTif_R16I() {
  u32 w, h;
  pixels = ::loadTif_R16I(path.string().c_str(), &w, &h, flipVertically);
  width = w;
  height = h;
  channels = 1;
}

void image2D::loadTif_R16UI() {
  u32 w, h;
  pixels = ::loadTif_R16UI(path.string().c_str(), &w, &h, flipVertically);
  width = w;
  height = h;
  channels = 1;
}

void image2D::clear() {
  switch (loadType) {
    case IMAGE2D_LOAD_NO:
      if (pixels)
        error("[image2D::clear] pixels is not nullptr");
      break;
    case IMAGE2D_LOAD_STB:
      stbi_image_free(pixels);
      break;
    case IMAGE2D_LOAD_R16I:
      delete[] (s16*)pixels;
      break;
    case IMAGE2D_LOAD_R16UI:
      delete[] (u16*)pixels;
      break;
    default:
      error("[image2D::clear] Should never happen [{}]", loadType);
  }
  pixels = nullptr;
  loadType = IMAGE2D_LOAD_NO;
}

