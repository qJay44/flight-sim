#include "GenerationManager.hpp"

#include "../texture/Texture2DArray.hpp"
#include "global.hpp"

namespace gfx::terrain {

using namespace core::math::terrain;

GenerationManager::GenerationManager(gfx::AssetManager& assetManager, u16 textureSize, int maxSlots) : maxSlots(maxSlots) {
  constexpr uvec2 localSize(16);

  textureSize += 2;

  assetManager.addShader("TerrainHeightCompute", gfx::Shader("terrain/height.comp"));
  assetManager.addShader("TerrainNormalsCompute", gfx::Shader("terrain/normals.comp"));
  assetManager.addShader("TerrainSwap", gfx::Shader("terrain/swap.comp"));
  assetManager.addTexture("TerrainNodes", Texture2DArray(maxSlots, ivec2{textureSize}, {.target = GL_TEXTURE_2D_ARRAY, .internalFormat = GL_RGBA32F, .format = GL_RGBA}));
  assetManager.addTexture("TerrainNodesDummy", Texture2DArray(maxSlots, ivec2{textureSize}, {.target = GL_TEXTURE_2D_ARRAY, .internalFormat = GL_RGBA32F, .format = GL_RGBA}));

  heightShader = assetManager.getShader("TerrainHeightCompute");
  normalsShader = assetManager.getShader("TerrainNormalsCompute");
  swapShader = assetManager.getShader("TerrainSwap");
  texArrayNodes = assetManager.getTexture("TerrainNodes");
  texArrayNodesDummy = assetManager.getTexture("TerrainNodesDummy");
  numGroups = (uvec2(textureSize) + localSize - 1u) / localSize;

  for (int i = 0; i < maxSlots; i++)
    freeSlots.push(i);

  ubo.terrainConfig = gfx::BufferObject::createUniformBuffer(true);
  ubo.terrainConfig.storage(&cfgTerrain, sizeof(TerrainConfig), GL_DYNAMIC_STORAGE_BIT);

  computeCommandHeight = {
    .shader = heightShader,
    .numWorkGroups = uvec3(numGroups, 0),
    .images = {
      ImageDescriptor{
      .texture = texArrayNodes,
      .access = GL_WRITE_ONLY,
      .format = GL_RGBA32F,
      .layererd = GL_TRUE,
    }},
  };

  computeCommandNormal = {
    .shader = normalsShader,
    .numWorkGroups = uvec3(numGroups, 0),
    .images = {
      ImageDescriptor{
      .texture = texArrayNodes,
      .access = GL_READ_ONLY,
      .format = GL_RGBA32F,
      .layererd = GL_TRUE
      },
      ImageDescriptor{
      .texture = texArrayNodesDummy,
      .access = GL_WRITE_ONLY,
      .format = GL_RGBA32F,
      .layererd = GL_TRUE
      },
    },
  };

  computeCommandSwap = {
    .shader = swapShader,
    .numWorkGroups = uvec3(numGroups, 0),
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

void GenerationManager::generateTexures(size_t nodesCount, size_t offset, Renderer& renderer, const BufferObject& nodes, float planetRadius, float heightScale) {
  computeCommandHeight.numWorkGroups.z = nodesCount;
  computeCommandNormal.numWorkGroups.z = nodesCount;
  computeCommandSwap  .numWorkGroups.z = nodesCount;

  heightShader->setUniform1f("u_planetRadius", planetRadius);
  heightShader->setUniform1f("u_heightScale", heightScale);
  heightShader->setUniform1ui("u_offset", offset);
  normalsShader->setUniform1ui("u_offset", offset);
  swapShader->setUniform1ui("u_offset", offset);
  ubo.terrainConfig.bindBase(0);
  nodes.bindBase(1);

  renderer.submit(computeCommandHeight);
  renderer.dispatch();
  renderer.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  renderer.submit(computeCommandNormal);
  renderer.dispatch();
  renderer.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  renderer.submit(computeCommandSwap);
  renderer.dispatch();
  renderer.memoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

} // terrain

