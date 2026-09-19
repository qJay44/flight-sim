#include "GenerationManager.hpp"

#include "../texture/Texture2DArray.hpp"
#include "global.hpp"

namespace gfx::terrain {

GenerationManager::GenerationManager(gfx::AssetManager& assetManager, int textureSize, int maxSlots) : maxSlots(maxSlots) {
  assetManager.addShader("TerrainHeightCompute", gfx::Shader("terrain/height.comp"));
  assetManager.addShader("TerrainNormalsCompute", gfx::Shader("terrain/normals.comp"));
  assetManager.addShader("TerrainSwap", gfx::Shader("terrain/swap.comp"));
  assetManager.addTexture("TerrainNodes", Texture2DArray(maxSlots, ivec2{textureSize + 2}, {.target = GL_TEXTURE_2D_ARRAY, .internalFormat = GL_RGBA32F, .format = GL_RGBA}));
  assetManager.addTexture("TerrainNodesDummy", Texture2DArray(maxSlots, ivec2{textureSize + 2}, {.target = GL_TEXTURE_2D_ARRAY, .internalFormat = GL_RGBA32F, .format = GL_RGBA}));

  heightShader = assetManager.getShader("TerrainHeightCompute");
  normalsShader = assetManager.getShader("TerrainNormalsCompute");
  swapShader = assetManager.getShader("TerrainSwap");
  texArrayNodes = assetManager.getTexture("TerrainNodes");
  texArrayNodesDummy = assetManager.getTexture("TerrainNodesDummy");
  numGroups = textureSize / 16;

  for (int i = 0; i < maxSlots; i++)
    freeSlots.push(i);

  ubo.terrainConfig.gen();
  ubo.terrainConfig.storage(&cfgTerrain, sizeof(TerrainConfig), GL_DYNAMIC_STORAGE_BIT);

  global::json::loadPreset(cfgTerrain, "heightmap1.json");
}

GenerationManager::TerrainConfig& GenerationManager::getConfig() {
  return cfgTerrain;
}

void GenerationManager::update() {
  ubo.terrainConfig.updateSubData(&cfgTerrain, sizeof(TerrainConfig));
}

int GenerationManager::acquireSlot() {
  assert(!freeSlots.empty());
  int slot = freeSlots.top();
  freeSlots.pop();

  return slot;
}

void GenerationManager::freeSlot(int slot) {
  assert((int)freeSlots.size() < maxSlots);
  freeSlots.push(slot);
}

void GenerationManager::freeSlotAll() {
  while (!freeSlots.empty())
    freeSlots.pop();

  for (int i = 0; i < maxSlots; i++)
    freeSlots.push(i);
}

void GenerationManager::generateTerrain(const core::math::terrain::NodeData& node, float planetRadius, float heightScale) {
  heightShader->use();
  heightShader->setUniform2f("u_nodeCenter", node.center);
  heightShader->setUniform1f("u_nodeExtents", node.extents);
  heightShader->setUniform1f("u_planetRadius", planetRadius);
  heightShader->setUniform1f("u_heightScale", heightScale);
  heightShader->setUniform1i("u_nodeFaceIdx", node.faceIdx);
  heightShader->setUniform1i("u_layer", node.texLayerIdx);

  ubo.terrainConfig.bindBase(0);
  glBindImageTexture(0, texArrayNodes->getId(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glDispatchCompute(numGroups, numGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  normalsShader->use();
  normalsShader->setUniform1i("u_layer", node.texLayerIdx);
  glBindImageTexture(0, texArrayNodes->getId(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
  glBindImageTexture(1, texArrayNodesDummy->getId(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glDispatchCompute(numGroups, numGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  swapShader->use();
  swapShader->setUniform1i("u_layer", node.texLayerIdx);
  glBindImageTexture(0, texArrayNodesDummy->getId(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
  glBindImageTexture(1, texArrayNodes->getId(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glDispatchCompute(numGroups, numGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

} // terrain

