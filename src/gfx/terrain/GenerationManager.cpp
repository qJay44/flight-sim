#include "GenerationManager.hpp"

#include "../texture/Texture2DArray.hpp"
#include "Tessendorf.hpp"
#include "global.hpp"

namespace gfx::terrain {

#define TEXTURE_MAX_SLOTS TERRAIN_MAX_NODES

GenerationManager::GenerationManager(ProfilerManager* profiler, AssetManager& assetManager) : profiler(profiler) {
  constexpr uvec2 localSize(16);
  constexpr ivec2 textureResNodes(160);
  constexpr ivec2 textureResOpticalDepth(256);
  constexpr ivec2 textureResNodesDummy = textureResNodes + 2;

  water = water::Tessendorf(assetManager);
  water.markForRebuild();

  gfx::TextureDescriptor texDescNodes{
    .target = GL_TEXTURE_2D_ARRAY,
    .internalFormat = GL_RGBA32F,
    .format = GL_RGBA,
  };

  assetManager.addShader("TerrainComputeHeight", gfx::Shader("terrain/height.comp"));
  assetManager.addShader("TerrainComputeNormals", gfx::Shader("terrain/normals.comp"));
  assetManager.addShader("TerrainOpticalDepth", gfx::Shader("terrain/optical-depth.comp"));

  assetManager.addTexture("TerrainNodes", Texture2DArray(TEXTURE_MAX_SLOTS, textureResNodes, texDescNodes));
  assetManager.addTexture("TerrainNodesDummy", Texture2DArray(TEXTURE_MAX_SLOTS, textureResNodesDummy, texDescNodes));
  assetManager.addTexture("TerrainOpticalDepth", Texture2D(textureResOpticalDepth, {.internalFormat = GL_RG32F, .format = GL_RG}));

  shaderHeight  = assetManager.getShader("TerrainComputeHeight");
  shaderNormals = assetManager.getShader("TerrainComputeNormals");
  shaderOpticalDepth = assetManager.getShader("TerrainOpticalDepth");

  texArrayNodes = assetManager.getTexture("TerrainNodes");
  texArrayNodesDummy = assetManager.getTexture("TerrainNodesDummy");
  texOpticalDepth = assetManager.getTexture("TerrainOpticalDepth");

  for (int i = 0; i < TEXTURE_MAX_SLOTS; i++)
    freeSlots.push(i);

  cfgAtmosphere.scatteringStrengthR = 0.002f;
  cfgAtmosphere.scatteringStrengthM = 0.0001f;
  cfgAtmosphere.heightScale = 2.f;
  cfgAtmosphere.densityFalloffR = 27.f;
  cfgAtmosphere.densityFalloffM = 44.f;

  ubo.terrainConfig = gfx::BufferObject::createUniformBuffer(true);
  ubo.terrainConfig.storage(&cfgTerrain, sizeof(TerrainConfig), GL_DYNAMIC_STORAGE_BIT);

  uvec2 numGroupsMain = (uvec2(textureResNodes) + localSize - 1u) / localSize;
  uvec2 numGroupsDummy = (uvec2(textureResNodesDummy) + localSize - 1u) / localSize;
  uvec2 numGroupsOpticalDepth = (uvec2(textureResOpticalDepth) + localSize - 1u) / localSize;

  computeCommandHeight = {
    .shader = shaderHeight,
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
    .shader = shaderNormals,
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

  computeCommandOpticalDepth = {
    .shader = shaderOpticalDepth,
    .numWorkGroups = uvec3(numGroupsOpticalDepth, 1),
    .images = {
      ImageDescriptor{
        .texture = texOpticalDepth,
        .access = GL_WRITE_ONLY,
        .format = GL_RG32F,
      }
    }
  };

  global::json::loadPreset(cfgTerrain, "heightmap4.json");
}

GenerationManager::TerrainConfig& GenerationManager::getTerrainConfig() {
  return cfgTerrain;
}

GenerationManager::AtmosphereConfig& GenerationManager::getAtmosphereConfig() {
  return cfgAtmosphere;
}

water::Tessendorf& GenerationManager::getWater() {
  return water;
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

void GenerationManager::markBakeOpticalDepth() {
  rebakeOpticalDepth = true;
}

void GenerationManager::update(Renderer& renderer, float time, float dt) {
  ubo.terrainConfig.updateSubData(&cfgTerrain, sizeof(TerrainConfig));
  water.update(renderer, time, dt);

  if (rebakeOpticalDepth)
    bakeOpticalDepth(renderer);
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

void GenerationManager::generateTexures(Renderer& renderer, size_t nodesCount, size_t offset, const BufferObject& nodes) {
  computeCommandHeight.numWorkGroups.z = nodesCount;
  computeCommandNormals.numWorkGroups.z = nodesCount;

  float heightScale = cfgTerrain.planetRadius * cfgTerrain.planetRadiusPercent;

  shaderHeight->setUniform1f("u_heightScale", heightScale);
  shaderHeight->setUniform1ui("u_offset", offset);
  shaderNormals->setUniform1ui("u_offset", offset);

  ubo.terrainConfig.bindBase(0);
  nodes.bindBase(1);

  renderer.submit(computeCommandHeight);
  renderer.dispatch();
  renderer.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  renderer.submit(computeCommandNormals);
  renderer.dispatch();
  renderer.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void GenerationManager::recalcScatteringCoefficients() {
  cfgAtmosphere.scatteringCoefficientsR = glm::pow(400.f / cfgAtmosphere.waveLengths, vec3(4.f)) * cfgAtmosphere.scatteringStrengthR;
  cfgAtmosphere.scatteringCoefficientsM = glm::pow(400.f / cfgAtmosphere.waveLengths, vec3(4.f)) * cfgAtmosphere.scatteringStrengthM;
}

void GenerationManager::bakeOpticalDepth(Renderer& renderer) {
  shaderOpticalDepth->setUniform3f("u_planetCenter", vec3(0.f));
  shaderOpticalDepth->setUniform1f("u_planetRadius", cfgTerrain.planetRadius);
  shaderOpticalDepth->setUniform1f("u_atmosphereRadius", cfgTerrain.planetRadius * cfgAtmosphere.heightScale);
  shaderOpticalDepth->setUniform1f("u_densityFalloffR", cfgAtmosphere.densityFalloffR);
  shaderOpticalDepth->setUniform1f("u_densityFalloffM", cfgAtmosphere.densityFalloffM);
  shaderOpticalDepth->setUniform1i("u_opticalDepthPoints", cfgAtmosphere.opticalDepthPoints);

  renderer.submit(computeCommandOpticalDepth);
  renderer.dispatch();
  renderer.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

  rebakeOpticalDepth = false;
}

} // terrain

