#pragma once

#include "../BufferObject.hpp"
#include "../Shader.hpp"
#include "../AssetManager.hpp"
#include "../../core/math/terrain/NodeData.hpp"
#include "nlohmann/json.hpp"

namespace gfx::terrain {

class GenerationManager {
public:
  struct TerrainConfig {
    float landThresholdA = 0.42f;
    float landThresholdB = 0.55f;
    float continentFreq = 1.2f;
    float initAmplitude = 0.5f;
    float initFrequency = 1.0f;
    float gain = 0.5f;
    float lacunarity = 2.0f;
    float canyonSteps = 15.f;
    float fbmOffsetFreq1 = 3.f;
    float fbmOffsetFreq2 = 3.f;
    float fbmOffsetFreq3 = 3.f;
    float fbmOffsetTwist = 0.25f;
    float f1VoronoiFreq1 = 5.f;
    float f1VoronoiFreq2 = 10.5f;
    float f1f2VoronoiFreq1 = 5.f;
    float f1f2VoronoiFreq2 = 10.5f;
    float detailInitAmplitude = 0.5f;
    float detailInitFrequency = 12.0f;
    float detailGain = 0.48f;
    float detailLacunarity = 2.1f;
    int octaves = 2;
    int detailOctaves = 10;
    float _pad[2];
  };
  static_assert(sizeof(TerrainConfig) % 16 == 0);

  GenerationManager(gfx::AssetManager& assetManager, int textureSize, int maxSlots);

  GenerationManager(GenerationManager&&) = default;
  GenerationManager& operator=(GenerationManager&&) = default;

  GenerationManager(const GenerationManager&) = delete;
  GenerationManager& operator=(const GenerationManager&) = delete;

  TerrainConfig& getConfig();

  void update();

  [[nodiscard]] int acquireSlot();
  void freeSlot(int slot);
  void freeSlotAll();
  void generateTerrain(const core::math::terrain::NodeData& node, float planetRadius, float heightScale);

private:
  int maxSlots;

  gfx::Texture* texArrayNodes;
  gfx::Texture* texArrayNodesDummy;
  gfx::Shader* heightShader{};
  gfx::Shader* normalsShader{};
  gfx::Shader* swapShader{};

  GLuint numGroups = 0;
  std::stack<int> freeSlots;

  TerrainConfig cfgTerrain;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TerrainConfig,
    landThresholdA,
    landThresholdB,
    continentFreq,
    initAmplitude,
    initFrequency,
    gain,
    lacunarity,
    canyonSteps,
    fbmOffsetFreq1,
    fbmOffsetFreq2,
    fbmOffsetFreq3,
    fbmOffsetTwist,
    f1VoronoiFreq1,
    f1VoronoiFreq2,
    f1f2VoronoiFreq1,
    f1f2VoronoiFreq2,
    detailInitAmplitude,
    detailInitFrequency,
    detailGain,
    detailLacunarity,
    octaves,
    detailOctaves
  );

  struct {
    gfx::BufferObject terrainConfig{GL_UNIFORM_BUFFER, false};
  } ubo;
};

} // terrain

