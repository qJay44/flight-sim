#include "GenerationManager.hpp"

#include "../texture/Texture2DArray.hpp"
#include "global.hpp"

namespace gfx::terrain {

#define TEXTURE_RESOLUTION 160
#define TEXTURE_MAX_SLOTS TERRAIN_MAX_NODES

GenerationManager::GenerationManager(gfx::AssetManager& assetManager) {
  constexpr uvec2 localSize(16);

  gfx::TextureDescriptor texDesc{
    .target = GL_TEXTURE_2D_ARRAY,
    .internalFormat = GL_RGBA32F,
    .format = GL_RGBA,
  };

  assetManager.addShader("TerrainCompute", gfx::Shader("terrain/terrain.comp"));
  assetManager.addTexture("TerrainNodes", Texture2DArray(TEXTURE_MAX_SLOTS, TEXTURE_RESOLUTION, texDesc));

  genShader = assetManager.getShader("TerrainCompute");
  texArrayNodes = assetManager.getTexture("TerrainNodes");
  numGroups = (uvec2(TEXTURE_RESOLUTION) + localSize - 1u) / localSize;

  for (int i = 0; i < TEXTURE_MAX_SLOTS; i++)
    freeSlots.push(i);

  ubo.terrainConfig = gfx::BufferObject::createUniformBuffer(true);
  ubo.terrainConfig.storage(&cfgTerrain, sizeof(TerrainConfig), GL_DYNAMIC_STORAGE_BIT);

  computeCommandHeight = {
    .shader = genShader,
    .numWorkGroups = uvec3(numGroups, 0),
    .images = {
      ImageDescriptor{
      .texture = texArrayNodes,
      .access = GL_WRITE_ONLY,
      .format = GL_RGBA32F,
      .layererd = GL_TRUE,
    }},
  };

  global::json::loadPreset(cfgTerrain, "heightmap0.json");
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
  assert((int)freeSlots.size() < TEXTURE_MAX_SLOTS);
  freeSlots.push(slot);
}

void GenerationManager::freeSlotAll() {
  while (!freeSlots.empty())
    freeSlots.pop();

  for (int i = 0; i < TEXTURE_MAX_SLOTS; i++)
    freeSlots.push(i);
}

void GenerationManager::generateTexures(size_t nodesCount, size_t offset, Renderer& renderer, const BufferObject& nodes) {
  computeCommandHeight.numWorkGroups.z = nodesCount;

  float heightScale = cfgTerrain.planetRadius * cfgTerrain.planetRadiusPercent;

  genShader->setUniform1f("u_heightScale", heightScale);
  genShader->setUniform1ui("u_offset", offset);

  ubo.terrainConfig.bindBase(0);
  nodes.bindBase(1);

  renderer.submit(computeCommandHeight);
  renderer.dispatch();
  // renderer.memoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

} // terrain

