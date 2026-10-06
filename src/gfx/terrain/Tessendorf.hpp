#pragma once

#include "../Shader.hpp"
#include "../texture/Texture2D.hpp"
#include "../BufferObject.hpp"
#include "../Renderer.hpp"
#include "../AssetManager.hpp"
#include "ProfilerManager.hpp"
#include "nlohmann/json.hpp"

namespace gfx::terrain::water {

class Tessendorf {
public:
  struct SpectrumSettingsGUI {
    float scale;
    float windSpeed;
    float windDir;
    float fetch;
    float spreadBlend;
    float swell;
    float peakEnhancemnt;
    float shortWavesFade;
  };

  struct WaterConfig {
    float worldSize = 256.f;

    float seed1 = 13.37f;
    float seed2 = 42.f;

    float g = 9.81f;
    float depth = 500.f;
    float lengthScale = 5;
    float lambda = 1.f;

    int size = 256;
    int logSize;
    GLuint numWorkGroups;

    SpectrumSettingsGUI local{
      .scale = 1.f,
      .windSpeed = 0.5f,
      .windDir = glm::radians(-29.81f),
      .fetch = 1e5f,
      .spreadBlend = 1.f,
      .swell = 0.198f,
      .peakEnhancemnt = 3.3,
      .shortWavesFade = 0.01f
    };

    SpectrumSettingsGUI swell{
      .scale = 0.f,
      .windSpeed = 1.f,
      .windDir = 0.f,
      .fetch = 3e5f,
      .spreadBlend = 1.f,
      .swell = 1.f,
      .peakEnhancemnt = 3.3,
      .shortWavesFade = 0.01f
    };
  };

  Tessendorf(Tessendorf&&) = default;
  Tessendorf& operator=(Tessendorf&&) = default;

  Tessendorf(const Tessendorf&) = delete;
  Tessendorf& operator=(const Tessendorf&) = delete;

  Tessendorf() = default;
  Tessendorf(Renderer* renderer, ProfilerManager* profiler, AssetManager& assetManager);

  WaterConfig& getConfig();

  Texture2D& getTexDisplacement();
  Texture2D& getTexDerivatives();
  Texture2D& getTexTurbulence();

  void updateInitials();
  void update(float time, float dt);

  void markForRebuild();
  void markForRebuildShaderCommands();

private:
  struct SpectrumSettings {
    float scale;
    float angle;
    float spreadBlend;
    float swell;
    float alpha;
    float peakOmega;
    float gamma;
    float shortWavesFade;
  };
  static_assert(sizeof(SpectrumSettings) % 16 == 0);

  struct {
    BufferObject spectrums;
  } ubo;

  Renderer* renderer{};
  ProfilerManager* profiler{};

  WaterConfig cfg{};
  SpectrumSettings spectrums[2];

  Shader* shaderButterfly        {};
  Shader* shaderNoise            {};
  Shader* shaderInitialSpectrum  {};
  Shader* shaderConjugateSpectrum{};
  Shader* shaderTimeSpectrum     {};
  Shader* shaderIFFT_horizontal  {};
  Shader* shaderIFFT_vertical    {};
  Shader* shaderPermute          {};
  Shader* shaderMerge            {};

  Texture2D texButterfly;
  Texture2D texNoise;
  Texture2D texInitialSpectrum;
  Texture2D texPrecomputedData;
  Texture2D texBuffer;
  Texture2D texDxDz;
  Texture2D texDyDxz;
  Texture2D texDyxDyz;
  Texture2D texDxxDzz;
  Texture2D texDisplacement;
  Texture2D texDerivatives;
  Texture2D texTurbulence;

  Renderer::ComputeCommand cmdButterfly;
  Renderer::ComputeCommand cmdNoise;
  Renderer::ComputeCommand cmdSpectrumInit;
  Renderer::ComputeCommand cmdSpectrumConjugate;
  Renderer::ComputeCommand cmdSpectrumTimeEvo;
  Renderer::ComputeCommand cmdIFFT_horizontal;
  Renderer::ComputeCommand cmdIFFT_vertical;
  Renderer::ComputeCommand cmdPermute;
  Renderer::ComputeCommand cmdMerge;

  ProfilerManager::Query querieTimeEvolution{"Time evo pass"};
  ProfilerManager::Query querieIFFT{"IFFT pass"};
  ProfilerManager::Query querieMerge{"Merge pass"};
  ProfilerManager::Query querieDraw{"Draw pass"};

  bool rebuild = false;
  bool rebuildShaderCommands = false;

NLOHMANN_DEFINE_TYPE_INTRUSIVE(Tessendorf::SpectrumSettingsGUI,
  scale,
  windSpeed,
  windDir,
  fetch,
  spreadBlend,
  swell,
  peakEnhancemnt,
  shortWavesFade
);

NLOHMANN_DEFINE_TYPE_INTRUSIVE(Tessendorf::WaterConfig,
  worldSize,
  seed1,
  seed2,
  g,
  depth,
  lengthScale,
  lambda,
  local,
  swell
);

private:
  void build();
  void buildShadersCommands();

  static float JonswapAlpha(float g, float fetch, float windSpeed);
  static float JonswapPeakFrequency(float g, float fetch, float windSpeed);

  void fillSettings(const SpectrumSettingsGUI& display, SpectrumSettings& settings);

  void generateButterfly();
  void generateNoise();
  void generateInitialSpectrum();
  void generateWavesAtTime(float time);
  void generateIFFT(Texture2D& input, Texture2D& buffer);
  void generateMerge(float dt);
};

} // namespace gfx::terrain::water

