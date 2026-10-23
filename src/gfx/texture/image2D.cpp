#include "image2D.hpp"

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "utils/utils.hpp"

namespace gfx {

void info(fspath path, int& width, int& height, int& channels) {
  stbi_info(path.string().c_str(), &width, &height, &channels);
}

void image2D::write(fspath path, uvec2 size, u8 channels, byte* buf) {
  stbi_flip_vertically_on_write(true);
  stbi_write_png(path.string().c_str(), size.x, size.y, channels, buf, size.x * channels);
}

image2D::image2D(image2D&& other) {
  width = other.width;
  height = other.height;
  channels = other.channels;
  pixels = other.pixels;
  path = other.path;
  loadType = other.loadType;

  other.pixels = nullptr;
  other.loadType = LOAD_NO;
}

image2D& image2D::operator=(image2D&& other) {
  if (this != &other) {
    clear();
    width = other.width;
    height = other.height;
    channels = other.channels;
    pixels = other.pixels;
    path = other.path;
    loadType = other.loadType;

    other.pixels = nullptr;
    other.loadType = LOAD_NO;
  }

  return *this;
}

image2D::image2D(int width, int height) : width(width), height(height) {}

image2D::image2D(fspath path, const LoadDescriptor& desc) {
  load(path, desc);
}

image2D::~image2D() {
  clear();
}

void image2D::load(fspath path, const LoadDescriptor& desc) {
  clear();

  this->path = path;
  this->loadType = desc.loadType;

  switch (loadType) {
    case LOAD_NO:
      error("[image2D::load] What?");
      break;
    case LOAD_STB:
      load_STB(desc);
      break;
    case LOAD_STBF:
      load_STBF(desc);
      break;
    default:
      error("[image2D::load] Unexpected loadType ({})", loadType);
  }

  if (!pixels)
    error("[image2D::load] Didn't load image [{}]", path.string());
}

void image2D::load_STB(const LoadDescriptor& desc) {
  stbi_set_flip_vertically_on_load(desc.flipVertically);
  pixels = stbi_load(path.string().c_str(), &width, &height, &channels, desc.desiredChannels);
}

void image2D::load_STBF(const LoadDescriptor& desc) {
  stbi_set_flip_vertically_on_load(desc.flipVertically);
  pixels = stbi_loadf(path.string().c_str(), &width, &height, &channels, desc.desiredChannels);
}

void image2D::clear() {
  switch (loadType) {
    case LOAD_NO:
      if (pixels)
        error("[image2D::clear] pixels is not nullptr");
      break;
    case LOAD_STB:
    case LOAD_STBF:
      stbi_image_free(pixels);
      break;
    default:
      error("[image2D::clear] Should never happen [{}]", loadType);
  }
  pixels = nullptr;
  loadType = LOAD_NO;
}

} // namespace gfx

