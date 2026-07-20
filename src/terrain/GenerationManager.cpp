#include "GenerationManager.hpp"

#include "global.hpp"
#include "shared.hpp"
#include "Terrain.hpp"

#define MAX_SLOTS TERRAIN_MAX_NODES

namespace terrain {

bool GenerationManager::isInitialized = false;

GenerationManager::GenerationManager(int textureSize) {
  if (std::exchange(isInitialized, true))
    error("[GenerationManager::GenerationManager] Already initialized");

  terrainShader = Shader("terrain/terrain.comp");
  texArrayNodes = Texture2DArray(MAX_SLOTS, ivec2{textureSize}, {.target = GL_TEXTURE_2D_ARRAY, .internalFormat = GL_RGBA32F, .format = GL_RGBA});
  numGroups = textureSize / 16;

  queryGenerateTerrain = ProfilerManager::Query{"Terrain generate"};

  for (int i = 0; i < MAX_SLOTS; i++)
    freeSlots.push(i);

  ubo.terrainConfig.gen();
  ubo.terrainConfig.storage(&cfgTerrain, sizeof(TerrainConfig), GL_DYNAMIC_STORAGE_BIT);

  global::json::loadPreset(cfgTerrain, "heightmap1.json");
}

GenerationManager::~GenerationManager() {
  isInitialized = false;
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
  assert(freeSlots.size() < MAX_SLOTS);
  freeSlots.push(slot);
}

void GenerationManager::freeSlotAll() {
  while (!freeSlots.empty())
    freeSlots.pop();

  for (int i = 0; i < MAX_SLOTS; i++)
    freeSlots.push(i);
}

void GenerationManager::generateTerrain(const NodeData& node) {
  global::profiler.startScopedTaskGpu(queryGenerateTerrain);

  terrainShader.use();
  terrainShader.setUniform2f("u_nodeCenter", node.center);
  terrainShader.setUniform1f("u_nodeExtents", node.extents);
  terrainShader.setUniform1f("u_planetRadius", planetRadius);
  terrainShader.setUniform1f("u_heightScale", heightScale);
  terrainShader.setUniform1i("u_nodeFaceIdx", node.faceIdx);
  terrainShader.setUniform1i("u_layer", node.texLayerIdx);

  ubo.terrainConfig.bindBase(0);
  glBindImageTexture(0, texArrayNodes.getId(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glDispatchCompute(numGroups, numGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void GenerationManager::bindTextures(GLuint terrainTexSlot) const {
  texArrayNodes.bind(terrainTexSlot);
}

} // terrain

