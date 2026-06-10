#include "TextureManager.hpp"

namespace terrain {

Shader TextureManager::shaderBufferA;
Shader TextureManager::shaderBufferB;

TextureManager::TextureManager(GLsizei slots, ivec2 size) {
  if (!shaderBufferA.initialized()) {
    shaderBufferA = Shader("terrain/bufferA.comp");
    shaderBufferB = Shader("terrain/bufferB.comp");
  }

  bufferA = Texture2DArray(slots, size, TextureDescriptor{
    .target = GL_TEXTURE_2D_ARRAY,
    .internalFormat = GL_RGBA16F,
    .format = GL_RGBA,
  });

  bufferB = Texture2DArray(slots, size, TextureDescriptor{
    .target = GL_TEXTURE_2D_ARRAY,
    .internalFormat = GL_RGB16F,
    .format = GL_RGB,
  });

  constexpr uvec3 localSize{16, 16, 1};
  numWorkGroups = (uvec3(size, 1u) + localSize - 1u) / localSize;
}

void TextureManager::generate(vec2 offset, int slot) {
  updateBufferA(offset, slot);
  updateBufferB(offset, slot);
}

void TextureManager::bind() const {
  bufferA.bind(0);
  bufferB.bind(1);
}

void TextureManager::updateBufferA(vec2 offset, int slot) {
  shaderBufferA.use();
  shaderBufferA.setUniform2f("u_offset", offset);
  shaderBufferA.setUniform1i("u_slot", slot);
  glBindImageTexture(0, bufferA.getId(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
  glDispatchCompute(numWorkGroups.x, numWorkGroups.y, numWorkGroups.z);
}

void TextureManager::updateBufferB(vec2 offset, int slot) {
  shaderBufferB.use();
  shaderBufferB.setUniform2f("u_offset", offset);
  shaderBufferB.setUniform1i("u_slot", slot);
  glBindImageTexture(0, bufferB.getId(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
  glDispatchCompute(numWorkGroups.x, numWorkGroups.y, numWorkGroups.z);
}

} // terrain

