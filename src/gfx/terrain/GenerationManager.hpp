#pragma once

#include "../BufferObject.hpp"
#include "../Shader.hpp"
#include "../AssetManager.hpp"
#include "../../gfx/Renderer.hpp"
#include "nlohmann/json.hpp"

namespace gfx::terrain {

class GenerationManager {
public:
  struct TerrainConfig {
    float planetRadius = 1.f;
    float planetRadiusPercent = 0.02f;
    float globalScale = 15.f;
    float initAmplitude = 0.5f;
    float initFrequency = 1.f;
    float gain = 0.5f;
    float lacunarity = 2.f;
    float initAmplitudeDetail = 1.f;
    float initFrequencyDetail = 1.f;
    float gainDetail = 0.5f;
    float lacunarityDetail = 3.f;
    float f1VoronoiFreq = 2.f;
    float displaceStrength = 1.f;
    float continentFreq = 1.f;
    float f2f1VoronoiFreq;
    int octavesDisplace = 2;
    int terraceSteps = 10;
    float _pad[3];
  };
  static_assert(sizeof(TerrainConfig) % 16 == 0);

  GenerationManager(gfx::AssetManager& assetManager);

  GenerationManager(GenerationManager&&) = default;
  GenerationManager& operator=(GenerationManager&&) = default;

  GenerationManager(const GenerationManager&) = delete;
  GenerationManager& operator=(const GenerationManager&) = delete;

  TerrainConfig& getConfig();
  size_t getFreeSlots() const;
  size_t getCachedSlots() const;
  bool isSlotCached(u64 nodeKey) const;

  void update();

  [[nodiscard]] int acquireSlot(u64 nodeKey);
  void freeSlot(u64 nodeKey, int slot);
  void freeSlotAll();

  void generateTexures(size_t nodesCount, size_t offset, Renderer& renderer, const BufferObject& nodes);

private:
  gfx::Texture* texArrayNodesDummy;
  gfx::Texture* texArrayNodes;

  gfx::Shader* heightShader{};
  gfx::Shader* normalsShader{};

  std::stack<int> freeSlots;
  std::unordered_map<u64, int> cachedSlots;

  gfx::Renderer::ComputeCommand computeCommandHeight;
  gfx::Renderer::ComputeCommand computeCommandNormals;

  TerrainConfig cfgTerrain;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TerrainConfig,
    planetRadius,
    planetRadiusPercent,
    globalScale,
    initAmplitude,
    initFrequency,
    gain,
    lacunarity,
    initAmplitudeDetail,
    initFrequencyDetail,
    gainDetail,
    lacunarityDetail,
    f1VoronoiFreq,
    displaceStrength,
    continentFreq,
    f2f1VoronoiFreq,
    octavesDisplace,
    terraceSteps
  );

  struct {
    gfx::BufferObject terrainConfig;
  } ubo;
};

} // terrain

