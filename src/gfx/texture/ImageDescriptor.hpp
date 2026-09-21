#pragma once

#include "Texture.hpp"

namespace gfx {

struct ImageDescriptor {
  Texture* texture;
  GLenum access;
  GLenum format;
  GLint level = 0;
  GLboolean layererd = GL_FALSE;
  GLint layer = 0;
};

} // namespace gfx

