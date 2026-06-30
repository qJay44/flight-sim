#include "GenerationManager.hpp"

#include <cassert>

#include "../engine/Shader.hpp"
#include "Terrain.hpp"
#include "shared.hpp"

#define MAX_SLOTS TERRAIN_MAX_NODES

namespace terrain {

static Shader generateTextureShader;

GenerationManager::GenerationManager(int textureSize) {
  if (!generateTextureShader.initialized()) {
    generateTextureShader = Shader("terrain/terrain.comp");
    texArray = Texture2DArray(MAX_SLOTS, ivec2{textureSize}, {.target = GL_TEXTURE_2D_ARRAY, .internalFormat = GL_RGBA32F, .format = GL_RGBA});
    numGroups = textureSize / 16;
  }

  for (int i = 0; i < MAX_SLOTS; i++)
    freeSlots.push(i);

  ubo.config.gen();
  ubo.config.storage(&cfg, sizeof(Config), GL_DYNAMIC_STORAGE_BIT);
}

void GenerationManager::update() {
  ubo.config.updateSubData(&cfg, sizeof(Config));
}

int GenerationManager::acquireSlot() {
  assert(!freeSlots.empty());
  int slot = freeSlots.top();
  freeSlots.pop();

  return slot;
}

void GenerationManager::freeSlot(int slot) {
  assert(freeSlots.size() < MAX_SLOTS);
  freeSlots.push(slot);
}

void GenerationManager::freeSlotAll() {
  while (!freeSlots.empty())
    freeSlots.pop();

  for (int i = 0; i < MAX_SLOTS; i++)
    freeSlots.push(i);
}

void GenerationManager::generate(const NodeData& node) {
  generateTextureShader.use();
  generateTextureShader.setUniform2f("u_nodeCenter", node.center);
  generateTextureShader.setUniform1f("u_nodeExtents", node.extents);
  generateTextureShader.setUniform1f("u_planetRadius", planetRadius);
  generateTextureShader.setUniform1f("u_heightScale", heightScale);
  generateTextureShader.setUniform1i("u_nodeFaceIdx", node.faceIdx);
  generateTextureShader.setUniform1i("u_layer", node.texLayerIdx);

  ubo.config.bindBase(0);
  glBindImageTexture(0, texArray.getId(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glDispatchCompute(numGroups, numGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void GenerationManager::bindTexture(GLuint slot) const {
  texArray.bind(slot);
}

} // terrain

