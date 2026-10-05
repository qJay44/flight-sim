#include "GenerationManager.hpp"

#include "../texture/Texture2DArray.hpp"
#include "global.hpp"

namespace gfx::terrain {

#define TEXTURE_MAX_SLOTS TERRAIN_MAX_NODES

GenerationManager::GenerationManager(gfx::AssetManager& assetManager) {
  constexpr uvec2 localSize(16);
  constexpr ivec2 textureRes(160);
  constexpr ivec2 textureResDummy = textureRes + 2;

  gfx::TextureDescriptor texDesc{
    .target = GL_TEXTURE_2D_ARRAY,
    .internalFormat = GL_RGBA32F,
    .format = GL_RGBA,
  };

  assetManager.addShader("TerrainComputeHeight", gfx::Shader("terrain/height.comp"));
  assetManager.addShader("TerrainComputeNormals", gfx::Shader("terrain/normals.comp"));
  assetManager.addTexture("TerrainNodes", Texture2DArray(TEXTURE_MAX_SLOTS, textureRes, texDesc));
  assetManager.addTexture("TerrainNodesDummy", Texture2DArray(TEXTURE_MAX_SLOTS, textureResDummy, texDesc));

  heightShader  = assetManager.getShader("TerrainComputeHeight");
  normalsShader = assetManager.getShader("TerrainComputeNormals");
  texArrayNodes = assetManager.getTexture("TerrainNodes");
  texArrayNodesDummy = assetManager.getTexture("TerrainNodesDummy");

  for (int i = 0; i < TEXTURE_MAX_SLOTS; i++)
    freeSlots.push(i);

  ubo.terrainConfig = gfx::BufferObject::createUniformBuffer(true);
  ubo.terrainConfig.storage(&cfgTerrain, sizeof(TerrainConfig), GL_DYNAMIC_STORAGE_BIT);

  uvec2 numGroupsMain = (uvec2(textureRes) + localSize - 1u) / localSize;
  uvec2 numGroupsDummy = (uvec2(textureResDummy) + localSize - 1u) / localSize;

  computeCommandHeight = {
    .shader = heightShader,
    .numWorkGroups = uvec3(numGroupsDummy, 0),
    .images = {
      ImageDescriptor{
      .texture = texArrayNodesDummy,
      .access = GL_WRITE_ONLY,
      .format = GL_RGBA32F,
      .layererd = GL_TRUE,
    }},
  };

  computeCommandNormals = {
    .shader = normalsShader,
    .numWorkGroups = uvec3(numGroupsMain, 0),
    .images = {
      ImageDescriptor{
      .texture = texArrayNodesDummy,
      .access = GL_READ_ONLY,
      .format = GL_RGBA32F,
      .layererd = GL_TRUE
      },
      ImageDescriptor{
      .texture = texArrayNodes,
      .access = GL_WRITE_ONLY,
      .format = GL_RGBA32F,
      .layererd = GL_TRUE
      },
    },
  };

  global::json::loadPreset(cfgTerrain, "heightmap0.json");
}

GenerationManager::TerrainConfig& GenerationManager::getConfig() {
  return cfgTerrain;
}

size_t GenerationManager::getFreeSlots() const {
  return freeSlots.size();
}

size_t GenerationManager::getCachedSlots() const {
  return cachedSlots.size();
}

bool GenerationManager::isSlotCached(u64 nodeKey) const {
  return cachedSlots.contains(nodeKey);
}

void GenerationManager::update() {
  ubo.terrainConfig.updateSubData(&cfgTerrain, sizeof(TerrainConfig));
}

int GenerationManager::acquireSlot(u64 nodeKey) {
  auto it = cachedSlots.find(nodeKey);
  if (it != cachedSlots.end()) {
    int slot = it->second;
    cachedSlots.erase(it);
    return slot;
  }

  int slot;

  if (freeSlots.empty()) {
    slot = cachedSlots.begin()->second;
    cachedSlots.erase(cachedSlots.begin()); // Might eat visible chunk?
  } else {
    slot = freeSlots.top();
    freeSlots.pop();
  }

  return slot;
}

void GenerationManager::freeSlot(u64 nodeKey, int slot) {
  assert(!cachedSlots.contains(slot));
  cachedSlots.emplace(nodeKey, slot);
}

void GenerationManager::freeSlotAll() {
  while (!freeSlots.empty())
    freeSlots.pop();

  for (int i = 0; i < TEXTURE_MAX_SLOTS; i++)
    freeSlots.push(i);

  cachedSlots.clear();
}

void GenerationManager::generateTexures(size_t nodesCount, size_t offset, Renderer& renderer, const BufferObject& nodes) {
  computeCommandHeight.numWorkGroups.z = nodesCount;
  computeCommandNormals.numWorkGroups.z = nodesCount;

  float heightScale = cfgTerrain.planetRadius * cfgTerrain.planetRadiusPercent;

  heightShader->setUniform1f("u_heightScale", heightScale);
  heightShader->setUniform1ui("u_offset", offset);
  normalsShader->setUniform1ui("u_offset", offset);

  ubo.terrainConfig.bindBase(0);
  nodes.bindBase(1);

  renderer.submit(computeCommandHeight);
  renderer.dispatch();
  renderer.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  renderer.submit(computeCommandNormals);
  renderer.dispatch();
  renderer.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

} // terrain

