#pragma once

#include <stack>

#include "../engine/texture/Texture2DArray.hpp"
#include "../engine/mesh/BufferObject.hpp"
#include "../engine/Shader.hpp"
#include "NodeData.hpp"
#include "ProfilerManager.hpp"
#include "nlohmann/json.hpp"

struct gui;

namespace terrain {

class GenerationManager {
public:
  GenerationManager() = default;
  GenerationManager(int textureSize);

  GenerationManager(GenerationManager&&) = default;
  GenerationManager& operator=(GenerationManager&&) = default;

  GenerationManager(const GenerationManager&) = delete;
  GenerationManager& operator=(const GenerationManager&) = delete;

  ~GenerationManager();

  void update();

  [[nodiscard]] int acquireSlot();
  void freeSlot(int slot);
  void freeSlotAll();

  void generateTerrain(const NodeData& node);
  void bindTextures(GLuint terrainTexSlot = 0) const;
private:
  friend ::gui;

  static bool isInitialized;

  Texture2DArray texArrayNodes;
  GLuint numGroups = 0;
  std::stack<int> freeSlots;

  Shader terrainShader;

  ProfilerManager::Query queryGenerateTerrain;

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
  } cfgTerrain;
  static_assert(sizeof(TerrainConfig) % 16 == 0);

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
    BufferObject terrainConfig{GL_UNIFORM_BUFFER, false};
  } ubo;
};

} // terrain

