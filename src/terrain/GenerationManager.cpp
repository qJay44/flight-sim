#include "GenerationManager.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <utility>

#include "Terrain.hpp"
#include "global.hpp"
#include "shared.hpp"

#define MAX_SLOTS TERRAIN_MAX_NODES
#define WATERMAP_SIZE 512

namespace terrain {

bool GenerationManager::isInitialized = false;

GenerationManager::GenerationManager(int textureSize) {
  if (std::exchange(isInitialized, true))
    error("[GenerationManager::GenerationManager] Already initialized");

  TextureDescriptor texWaterMapDesc{};
  texWaterMapDesc.internalFormat = GL_RGBA16F;
  texWaterMapDesc.format = GL_RGBA;
  texWaterMapDesc.wrapS = GL_REPEAT;
  texWaterMapDesc.wrapT = GL_REPEAT;

  terrainShader = Shader("terrain/terrain.comp");
  waterShader = Shader("terrain/water.comp");
  texArrayNodes = Texture2DArray(MAX_SLOTS, ivec2{textureSize}, {.target = GL_TEXTURE_2D_ARRAY, .internalFormat = GL_RGBA32F, .format = GL_RGBA});
  texWaterMap = Texture2D(ivec2(WATERMAP_SIZE), texWaterMapDesc);
  numGroups = textureSize / 16;

  queryGenerateTerrain = ProfilerManager::Query{"Terrain generate"};
  queryGenerateWater = ProfilerManager::Query{"Water generate"};

  for (int i = 0; i < MAX_SLOTS; i++)
    freeSlots.push(i);

  ubo.terrainConfig.gen();
  ubo.terrainConfig.storage(&cfgTerrain, sizeof(TerrainConfig), GL_DYNAMIC_STORAGE_BIT);

  ubo.waterConfig.gen();
  ubo.waterConfig.storage(&cfgWater, sizeof(WaterConfig), GL_DYNAMIC_STORAGE_BIT);
}

GenerationManager::~GenerationManager() {
  isInitialized = false;
}

void GenerationManager::update() {
  ubo.terrainConfig.updateSubData(&cfgTerrain, sizeof(TerrainConfig));
  ubo.waterConfig.updateSubData(&cfgWater, sizeof(WaterConfig));
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

void GenerationManager::generateWater() {
  global::profiler.startScopedTaskGpu(queryGenerateWater);

  waterShader.use();
  waterShader.setUniform1f("u_time", global::time);

  ubo.waterConfig.bindBase(0);
  glBindImageTexture(0, texWaterMap.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
  glDispatchCompute(WATERMAP_SIZE / 16, WATERMAP_SIZE / 16, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void GenerationManager::bindTextures(GLuint terrainTexSlot, GLuint waterTexSlot) const {
  texArrayNodes.bind(terrainTexSlot);
  texWaterMap.bind(waterTexSlot);
}

void GenerationManager::loadTerrainConfig(std::string_view name) {
  fspath path = fspath("res/data/cfg") / name;

  std::ifstream f(path);

  if (f.is_open()) {
    nlohmann::json j;
    f >> j;
    cfgTerrain = j.get<TerrainConfig>();
    f.close();
  } else {
    warning("[GenerationManager::loadConfig] Could not open the file [{}]", path.string());
  }
}

void GenerationManager::loadWaterConfig(std::string_view name) {
  fspath path = fspath("res/data/cfg") / name;

  std::ifstream f(path);

  if (f.is_open()) {
    nlohmann::json j;
    f >> j;
    cfgWater = j.get<WaterConfig>();
    f.close();
  } else {
    warning("[GenerationManager::loadConfig] Could not open the file [{}]", path.string());
  }
}

void GenerationManager::saveTerrainConfig(std::string_view name) const {
  fspath path = fspath("res/data/cfg") / name;
  std::filesystem::create_directories(path.parent_path());

  std::ofstream f(path);

  if (f.is_open()) {
    nlohmann::json j = cfgTerrain;
    f << j.dump(2) << std::endl;
    f.close();
  } else {
    error("[GenerationManager::saveConfig] Could not open the file [{}]", path.string());
  }
}

void GenerationManager::saveWaterConfig(std::string_view name) const {
  fspath path = fspath("res/data/cfg") / name;
  std::filesystem::create_directories(path.parent_path());

  std::ofstream f(path);

  if (f.is_open()) {
    nlohmann::json j = cfgWater;
    f << j.dump(2) << std::endl;
    f.close();
  } else {
    error("[GenerationManager::saveConfig] Could not open the file [{}]", path.string());
  }
}

} // terrain

