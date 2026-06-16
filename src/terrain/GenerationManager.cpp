#include "GenerationManager.hpp"

#include <cassert>

namespace terrain {

Shader GenerationManager::shaderBufferA;
Shader GenerationManager::shaderBufferB;

GenerationManager::GenerationManager(int resolution) : resolution(resolution) {
  if (!shaderBufferA.initialized()) {
    shaderBufferA = Shader("terrain/bufferA.comp");
    shaderBufferB = Shader("terrain/bufferB.comp");
  }

  ivec2 size{resolution};

  TextureDescriptor texDesc{};
  texDesc.target = GL_TEXTURE_2D;
  texDesc.internalFormat = GL_RGBA16F;
  texDesc.format = GL_RGBA;
  texDesc.wrapS = GL_REPEAT;
  texDesc.wrapT = GL_REPEAT;

  buffers[0][0] = Texture2D::storage(size, texDesc);
  buffers[0][1] = Texture2D::storage(size, texDesc);
  buffers[1][0] = Texture2D::storage(size, texDesc);
  buffers[1][1] = Texture2D::storage(size, texDesc);

  ubo.erosionConfig.allocate(&erosionConfig, sizeof(ErosionConfig), GL_DYNAMIC_DRAW);
}

const float& GenerationManager::getHeightmapScale() const {
  return heightmapScale;
}

void GenerationManager::setHeightmapScale(float s) {
  heightmapScale = s;
}

void GenerationManager::generateInit() {
  if (fence) {
    glDeleteSync(fence);
    fence = nullptr;
  }

  activeTask.dirtyX = ivec2(0, resolution);
  activeTask.dirtyY = ivec2(0, resolution);
  activeTask.pixelDelta = ivec2(1);
  activeTask.offset = vec2(0);
  currOffsetY = 0;
  sliceHeight = resolution;
  currReadIdx = 1;

  updateBufferA();
  updateBufferB();

  glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

  // Full wait
  GLsync fenceInit = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  glClientWaitSync(fenceInit, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
  glDeleteSync(fenceInit);

  sliceHeight = resolution / 16;
  currReadIdx = 0;
  status = IDLE;
}

void GenerationManager::generate(ivec2 pixelDelta, vec2 offset) {
  if (status == GENERATING)
    return;

  ivec2 dirtyX(0);
  ivec2 dirtyY(0);

  if (pixelDelta.x != 0) {
    int startX = texWriteHead.x;
    int endX = (texWriteHead.x + pixelDelta.x) % resolution;
    if (endX < 0) endX += resolution;
    dirtyX = ivec2{startX, endX};
  }

  if (pixelDelta.y != 0) {
    int startY = texWriteHead.y;
    int endY = (texWriteHead.y + pixelDelta.y) % resolution;
    if (endY < 0) endY += resolution;
    dirtyY = ivec2{startY, endY};
  }

  activeTask.dirtyX = dirtyX;
  activeTask.dirtyY = dirtyY;
  activeTask.pixelDelta = pixelDelta;
  activeTask.offset = offset;

  currOffsetY = 0;
  status = GENERATING;

  texWriteHead = (texWriteHead + pixelDelta) % resolution;
  if (texWriteHead.x < 0) texWriteHead.x += resolution;
  if (texWriteHead.y < 0) texWriteHead.y += resolution;
}

GenerationManager::Status GenerationManager::checkStatus() {
  if (status != GENERATING)
    return status = IDLE;

  if (fence) {
    GLenum fenceStatus = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
    if (fenceStatus != GL_ALREADY_SIGNALED && fenceStatus != GL_CONDITION_SATISFIED)
      return status;

    glDeleteSync(fence);
    fence = nullptr;

    currOffsetY += sliceHeight;
    if (currOffsetY >= resolution) {
      currOffsetY = 0;
      currReadIdx = 1 - currReadIdx;

      return status = DONE;
    }
  }

  updateBufferA();
  updateBufferB();

  glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

  fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  glFlush();

  return status = GENERATING;
}

void GenerationManager::bindTextures(GLuint slotA, GLuint slotB) const {
  buffers[currReadIdx][0].bind(slotA);
  buffers[currReadIdx][1].bind(slotB);
}

void GenerationManager::updateBufferA() {
  int writeIdx = 1 - currReadIdx;
  ubo.erosionConfig.updateSubData(&erosionConfig, sizeof(ErosionConfig));
  ubo.erosionConfig.bindBase(0);

  shaderBufferA.use();
  shaderBufferA.setUniform2i("u_dirtyX", activeTask.dirtyX);
  shaderBufferA.setUniform2i("u_dirtyY", activeTask.dirtyY);
  shaderBufferA.setUniform2i("u_pixelDelta", activeTask.pixelDelta);
  shaderBufferA.setUniform2f("u_offset", activeTask.offset);
  shaderBufferA.setUniform1f("u_heightmapScale", heightmapScale);
  shaderBufferA.setUniform1i("u_currOffsetY", currOffsetY);
  glBindImageTexture(0, buffers[currReadIdx][0].getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
  glBindImageTexture(1, buffers[writeIdx][0].getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
  glDispatchCompute(resolution / 16, sliceHeight / 16, 1);
}

void GenerationManager::updateBufferB() {
  int writeIdx = 1 - currReadIdx;

  shaderBufferB.use();
  shaderBufferB.setUniform2i("u_dirtyX", activeTask.dirtyX);
  shaderBufferB.setUniform2i("u_dirtyY", activeTask.dirtyY);
  shaderBufferB.setUniform2i("u_pixelDelta", activeTask.pixelDelta);
  shaderBufferB.setUniform2f("u_offset", activeTask.offset);
  shaderBufferB.setUniform1f("u_heightmapScale", heightmapScale);
  shaderBufferB.setUniform1i("u_currOffsetY", currOffsetY);
  glBindImageTexture(0, buffers[currReadIdx][1].getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
  glBindImageTexture(1, buffers[writeIdx][1].getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
  glDispatchCompute(resolution / 16, sliceHeight / 16, 1);
}

} // terrain

