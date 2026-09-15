#pragma once

namespace gfx {

// NOTE: Use GL_NEAREST for the integer samplers (isampler, usampler, etc.)
struct TextureDescriptor {
  GLenum target         = GL_TEXTURE_2D;
  GLenum internalFormat = GL_RGB8;          // Color format in the OpenGL
  GLenum format         = GL_RGB;           // Color format to treat as
  GLenum type           = GL_UNSIGNED_BYTE; // Pixels type
  GLenum minFilter      = GL_LINEAR;
  GLenum magFilter      = GL_LINEAR;
  GLenum wrapS          = GL_CLAMP_TO_EDGE;
  GLenum wrapT          = GL_CLAMP_TO_EDGE;
  GLenum wrapR          = GL_CLAMP_TO_EDGE;
};

} // namespace gfx

