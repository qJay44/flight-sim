#pragma once

#include "../../engine/Shader.hpp"
#include "../../engine/texture/Texture2D.hpp"
#include "../../engine/mesh/BufferObject.hpp"
#include "ProfilerManager.hpp"
#include "nlohmann/json.hpp"

struct gui;

namespace terrain::water {

class Tessendorf {
public:
  Tessendorf();

  void updateInitials();
  void update();

  void markForRebuild();

  void bindTextures(GLuint displacementUnit, GLuint derivativesUnit, GLuint turbulenceUnit) const;

private:
  friend ::gui;

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
    BufferObject spectrums{GL_UNIFORM_BUFFER};
  } ubo;

  static bool isInitialized;
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

  SpectrumSettings spectrums[2];

  Shader shaderButterfly         { "terrain/water/butterfly.comp"         };
  Shader shaderNoise             { "terrain/water/noise.comp"             };
  Shader shaderInitialSpectrum   { "terrain/water/initialSpectrum.comp"   };
  Shader shaderConjugateSpectrum { "terrain/water/conjugateSpectrum.comp" };
  Shader shaderTimeSpectrum      { "terrain/water/timeSpectrum.comp"      };
  Shader shaderIFFT_horizontal   { "terrain/water/ifft_horizontal.comp"   };
  Shader shaderIFFT_vertical     { "terrain/water/ifft_vertical.comp"     };
  Shader shaderPermute           { "terrain/water/permute.comp"           };
  Shader shaderMerge             { "terrain/water/merge.comp"             };

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

  ProfilerManager::Query querieTimeEvolution{"Time evo pass"};
  ProfilerManager::Query querieIFFT{"IFFT pass"};
  ProfilerManager::Query querieMerge{"Merge pass"};
  ProfilerManager::Query querieDraw{"Draw pass"};

  bool rebuild = false;

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

NLOHMANN_DEFINE_TYPE_INTRUSIVE(Tessendorf,
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

  static float JonswapAlpha(float g, float fetch, float windSpeed);
  static float JonswapPeakFrequency(float g, float fetch, float windSpeed);

  void fillSettings(const SpectrumSettingsGUI& display, SpectrumSettings& settings);

  void generateButterfly();
  void generateNoise();
  void generateInitialSpectrum();
  void generateWavesAtTime(float time);
  void generateIFFT(Texture2D& input, Texture2D& buffer);
  void generateMerge();
};

} // namespace water

