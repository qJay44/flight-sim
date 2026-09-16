#include "GenerationManager.hpp"

#include "../texture/Texture2DArray.hpp"
#include "global.hpp"

namespace gfx::terrain {

GenerationManager::GenerationManager(gfx::AssetManager& assetManager, int textureSize, int maxSlots) : maxSlots(maxSlots) {
  assetManager.addShader("TerrainCompute", gfx::Shader("terrain/terrain.comp"));
  assetManager.addTexture("TerrainNodes", Texture2DArray(maxSlots, ivec2{textureSize}, {.target = GL_TEXTURE_2D_ARRAY, .internalFormat = GL_RGBA32F, .format = GL_RGBA}));

  terrainShader = assetManager.getShader("TerrainCompute");
  texArrayNodes = assetManager.getTexture("TerrainNodes");
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
  terrainShader->use();
  terrainShader->setUniform2f("u_nodeCenter", node.center);
  terrainShader->setUniform1f("u_nodeExtents", node.extents);
  terrainShader->setUniform1f("u_planetRadius", planetRadius);
  terrainShader->setUniform1f("u_heightScale", heightScale);
  terrainShader->setUniform1i("u_nodeFaceIdx", node.faceIdx);
  terrainShader->setUniform1i("u_layer", node.texLayerIdx);

  ubo.terrainConfig.bindBase(0);
  glBindImageTexture(0, texArrayNodes->getId(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glDispatchCompute(numGroups, numGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

} // terrain

