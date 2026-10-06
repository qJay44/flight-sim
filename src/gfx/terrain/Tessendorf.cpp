#include "Tessendorf.hpp"

#include "glm/common.hpp"
#include "glm/exponential.hpp"
#include "global.hpp"

namespace gfx::terrain::water {

Tessendorf::Tessendorf(Renderer* renderer, ProfilerManager* profiler, AssetManager& assetManager)
  : renderer(renderer), profiler(profiler)
{
  {
    Shader _shaderButterfly         { "terrain/water/butterfly.comp"         };
    Shader _shaderNoise             { "terrain/water/noise.comp"             };
    Shader _shaderInitialSpectrum   { "terrain/water/initialSpectrum.comp"   };
    Shader _shaderConjugateSpectrum { "terrain/water/conjugateSpectrum.comp" };
    Shader _shaderTimeSpectrum      { "terrain/water/timeSpectrum.comp"      };
    Shader _shaderIFFT_horizontal   { "terrain/water/ifft_horizontal.comp"   };
    Shader _shaderIFFT_vertical     { "terrain/water/ifft_vertical.comp"     };
    Shader _shaderPermute           { "terrain/water/permute.comp"           };
    Shader _shaderMerge             { "terrain/water/merge.comp"             };

    assetManager.addShader("TessendorfButterfly",         std::move(_shaderButterfly        ));
    assetManager.addShader("TessendorfNoise",             std::move(_shaderNoise            ));
    assetManager.addShader("TessendorfInitialSpectrum",   std::move(_shaderInitialSpectrum  ));
    assetManager.addShader("TessendorfConjugateSpectrum", std::move(_shaderConjugateSpectrum));
    assetManager.addShader("TessendorfTimeSpectrum",      std::move(_shaderTimeSpectrum     ));
    assetManager.addShader("TessendorfIFFT_horizontal",   std::move(_shaderIFFT_horizontal  ));
    assetManager.addShader("TessendorfIFFT_vertical",     std::move(_shaderIFFT_vertical    ));
    assetManager.addShader("TessendorfPermute",           std::move(_shaderPermute          ));
    assetManager.addShader("TessendorfMerge",             std::move(_shaderMerge            ));
  }

  shaderButterfly         = assetManager.getShader("TessendorfButterfly");
  shaderNoise             = assetManager.getShader("TessendorfNoise");
  shaderInitialSpectrum   = assetManager.getShader("TessendorfInitialSpectrum");
  shaderConjugateSpectrum = assetManager.getShader("TessendorfConjugateSpectrum");
  shaderTimeSpectrum      = assetManager.getShader("TessendorfTimeSpectrum");
  shaderIFFT_horizontal   = assetManager.getShader("TessendorfIFFT_horizontal");
  shaderIFFT_vertical     = assetManager.getShader("TessendorfIFFT_vertical");
  shaderPermute           = assetManager.getShader("TessendorfPermute");
  shaderMerge             = assetManager.getShader("TessendorfMerge");

  ubo.spectrums = gfx::BufferObject::createUniformBuffer(true);
  ubo.spectrums.storage(nullptr, sizeof(spectrums), GL_DYNAMIC_STORAGE_BIT);
  build();

  global::json::loadPreset(cfg, "tessendorf0.json");
}

Tessendorf::WaterConfig& Tessendorf::getConfig() {
  return cfg;
}

Texture2D& Tessendorf::getTexDisplacement() {return texDisplacement;}
Texture2D& Tessendorf::getTexDerivatives() {return texDerivatives;}
Texture2D& Tessendorf::getTexTurbulence() {return texTurbulence;}

void Tessendorf::updateInitials() {
  generateButterfly();
  generateNoise();
  generateInitialSpectrum();
}

void Tessendorf::update(float time, float dt) {
  if (rebuild)
    build();

  if (rebuildShaderCommands)
    buildShadersCommands();

  generateWavesAtTime(time);
  generateIFFT(texDxDz, texBuffer);
  generateIFFT(texDyDxz, texBuffer);
  generateIFFT(texDyxDyz, texBuffer);
  generateIFFT(texDxxDzz, texBuffer);
  generateMerge(dt);
}

void Tessendorf::markForRebuild() {
  rebuild = true;
}

void Tessendorf::markForRebuildShaderCommands() {
  rebuildShaderCommands = true;
}

void Tessendorf::build() {
  TextureDescriptor descR    { .internalFormat = GL_R32F,    .format = GL_RED,  .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
  TextureDescriptor descRG   { .internalFormat = GL_RG32F,   .format = GL_RG,   .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
  TextureDescriptor descRGBA { .internalFormat = GL_RGBA32F, .format = GL_RGBA, .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
  cfg.logSize = glm::log2((float)cfg.size);
  cfg.numWorkGroups = cfg.size / 8;

  texButterfly       = Texture2D(ivec2(cfg.logSize, cfg.size), descRGBA);
  texNoise           = Texture2D(cfg.size, descRG);
  texInitialSpectrum = Texture2D(cfg.size, descRGBA);
  texPrecomputedData = Texture2D(cfg.size, descRGBA);
  texBuffer          = Texture2D(cfg.size, descRG);
  texDxDz            = Texture2D(cfg.size, descRG);
  texDyDxz           = Texture2D(cfg.size, descRG);
  texDyxDyz          = Texture2D(cfg.size, descRG);
  texDxxDzz          = Texture2D(cfg.size, descRG);
  texDisplacement    = Texture2D(cfg.size, descRGBA);
  texDerivatives     = Texture2D(cfg.size, descRGBA);
  texTurbulence      = Texture2D(cfg.size, descR);

  buildShadersCommands();
  updateInitials();

  rebuild = false;
}

void Tessendorf::buildShadersCommands() {
  cmdButterfly = {
    .shader = shaderButterfly,
    .numWorkGroups = uvec3(cfg.logSize, cfg.numWorkGroups / 2, 1),
    .images = {
      ImageDescriptor{
        .texture = &texButterfly,
        .access = GL_WRITE_ONLY,
        .format = GL_RGBA32F,
      }
    }
  };

  cmdNoise = {
    .shader = shaderNoise,
    .numWorkGroups = uvec3(cfg.numWorkGroups, cfg.numWorkGroups, 1),
    .images = {
      ImageDescriptor{
        .texture = &texNoise,
        .access = GL_WRITE_ONLY,
        .format = GL_RG32F,
      }
    }
  };

  cmdSpectrumInit = {
    .shader = shaderInitialSpectrum,
    .numWorkGroups = uvec3(cfg.numWorkGroups, cfg.numWorkGroups, 1),
    .images = {
      ImageDescriptor{
        .texture = &texNoise,
        .access = GL_READ_ONLY,
        .format = GL_RG32F,
      },
      ImageDescriptor{
        .texture = &texBuffer,
        .access = GL_WRITE_ONLY,
        .format = GL_RG32F,
      },
      ImageDescriptor{
        .texture = &texPrecomputedData,
        .access = GL_WRITE_ONLY,
        .format = GL_RGBA32F,
      }
    }
  };

  cmdSpectrumConjugate = {
    .shader = shaderConjugateSpectrum,
    .numWorkGroups = uvec3(cfg.numWorkGroups, cfg.numWorkGroups, 1),
    .images = {
      ImageDescriptor{
        .texture = &texBuffer,
        .access = GL_READ_ONLY,
        .format = GL_RG32F,
      },
      ImageDescriptor{
        .texture = &texInitialSpectrum,
        .access = GL_WRITE_ONLY,
        .format = GL_RGBA32F,
      }
    }
  };

  cmdSpectrumTimeEvo = {
    .shader = shaderTimeSpectrum,
    .numWorkGroups = uvec3(cfg.numWorkGroups, cfg.numWorkGroups, 1),
    .images = {
      ImageDescriptor{
        .texture = &texInitialSpectrum,
        .access = GL_READ_ONLY,
        .format = GL_RGBA32F,
      },
      ImageDescriptor{
        .texture = &texPrecomputedData,
        .access = GL_READ_ONLY,
        .format = GL_RGBA32F,
      },
      ImageDescriptor{
        .texture = &texDxDz,
        .access = GL_WRITE_ONLY,
        .format = GL_RG32F,
      },
      ImageDescriptor{
        .texture = &texDyDxz,
        .access = GL_WRITE_ONLY,
        .format = GL_RG32F,
      },
      ImageDescriptor{
        .texture = &texDyxDyz,
        .access = GL_WRITE_ONLY,
        .format = GL_RG32F,
      },
      ImageDescriptor{
        .texture = &texDxxDzz,
        .access = GL_WRITE_ONLY,
        .format = GL_RG32F,
      },
    }
  };

  cmdIFFT_horizontal = {
    .shader = shaderIFFT_horizontal,
    .numWorkGroups = uvec3(cfg.numWorkGroups, cfg.numWorkGroups, 1),
    .images = {
      ImageDescriptor{
        .texture = &texButterfly,
        .access = GL_READ_ONLY,
        .format = GL_RGBA32F,
      },
      ImageDescriptor{
        .texture = nullptr,
        .access = GL_READ_ONLY,
        .format = GL_RG32F,
      },
      ImageDescriptor{
        .texture = nullptr,
        .access = GL_WRITE_ONLY,
        .format = GL_RG32F,
      },
    }
  };

  cmdIFFT_vertical = cmdIFFT_horizontal;
  cmdIFFT_vertical.shader = shaderIFFT_vertical;

  cmdPermute = {
    .shader = shaderPermute,
    .numWorkGroups = uvec3(cfg.numWorkGroups, cfg.numWorkGroups, 1),
    .images = {
      ImageDescriptor{
        .texture = nullptr,
        .access = GL_READ_WRITE,
        .format = GL_RG32F,
      },
    }
  };

  cmdMerge = {
    .shader = shaderMerge,
    .numWorkGroups = uvec3(cfg.numWorkGroups, cfg.numWorkGroups, 1),
    .images = {
      ImageDescriptor{
        .texture = &texDxDz,
        .access = GL_READ_ONLY,
        .format = GL_RG32F,
      },
      ImageDescriptor{
        .texture = &texDyDxz,
        .access = GL_READ_ONLY,
        .format = GL_RG32F,
      },
      ImageDescriptor{
        .texture = &texDyxDyz,
        .access = GL_READ_ONLY,
        .format = GL_RG32F,
      },
      ImageDescriptor{
        .texture = &texDxxDzz,
        .access = GL_READ_ONLY,
        .format = GL_RG32F,
      },
      ImageDescriptor{
        .texture = &texDisplacement,
        .access = GL_WRITE_ONLY,
        .format = GL_RGBA32F,
      },
      ImageDescriptor{
        .texture = &texDerivatives,
        .access = GL_WRITE_ONLY,
        .format = GL_RGBA32F,
      },
      ImageDescriptor{
        .texture = &texTurbulence,
        .access = GL_READ_WRITE,
        .format = GL_R32F,
      },
    }
  };

  rebuildShaderCommands = false;
}

float Tessendorf::JonswapAlpha(float g, float fetch, float windSpeed) {
  return 0.076f * glm::pow(g * fetch / windSpeed / windSpeed, -0.22f);
}

float Tessendorf::JonswapPeakFrequency(float g, float fetch, float windSpeed) {
  return 22.f * glm::pow(windSpeed * fetch / g / g, -0.33f);
}

void Tessendorf::fillSettings(const SpectrumSettingsGUI& display, SpectrumSettings& settings) {
  settings.scale = display.scale;
  settings.angle = display.windDir;
  settings.spreadBlend = display.spreadBlend;
  settings.swell = glm::clamp(display.swell, 0.01f, 1.f);
  settings.alpha = JonswapAlpha(cfg.g, display.fetch, display.windSpeed);
  settings.peakOmega = JonswapPeakFrequency(cfg.g, display.fetch, display.windSpeed);
  settings.gamma = display.peakEnhancemnt;
  settings.shortWavesFade = display.shortWavesFade;
}

void Tessendorf::generateButterfly() {
  shaderButterfly->setUniform1i("u_size", cfg.size);
  renderer->submit(cmdButterfly);
  renderer->dispatch();
  renderer->memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Tessendorf::generateNoise() {
  shaderNoise->setUniform1f("u_seed1", cfg.seed1);
  shaderNoise->setUniform1f("u_seed2", cfg.seed2);
  renderer->submit(cmdNoise);
  renderer->dispatch();
  renderer->memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Tessendorf::generateInitialSpectrum() {
  fillSettings(cfg.local, spectrums[0]);
  fillSettings(cfg.swell, spectrums[1]);
  ubo.spectrums.updateSubData(spectrums, sizeof(spectrums));

  ubo.spectrums.bindBase(0);
  texNoise.bind(0);

  shaderInitialSpectrum->setUniform1f("u_g", cfg.g);
  shaderInitialSpectrum->setUniform1f("u_depth", cfg.depth);
  shaderInitialSpectrum->setUniform1f("u_lengthScale", cfg.lengthScale);
  renderer->submit(cmdSpectrumInit);
  renderer->dispatch();
  renderer->memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  renderer->submit(cmdSpectrumConjugate);
  renderer->dispatch();
  renderer->memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Tessendorf::generateWavesAtTime(float time) {
  auto _task = profiler->startScopedTaskGpu(querieTimeEvolution);

  shaderTimeSpectrum->setUniform1f("u_time", time);
  renderer->submit(cmdSpectrumTimeEvo);
  renderer->dispatch();
  renderer->memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Tessendorf::generateIFFT(Texture2D& input, Texture2D& buffer) {
  auto _task = profiler->startScopedTaskGpu(querieIFFT);

  cmdIFFT_horizontal.images[1].texture = &input;
  cmdIFFT_horizontal.images[2].texture = &buffer;

  for (int i = 0; i < cfg.logSize; i++) {
    shaderIFFT_horizontal->setUniform1i("u_step", i);
    renderer->submit(cmdIFFT_horizontal);
    renderer->dispatch();
    renderer->memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    std::swap(cmdIFFT_horizontal.images[1], cmdIFFT_horizontal.images[2]);
  }

  cmdIFFT_vertical.images[1].texture = cmdIFFT_horizontal.images[1].texture;
  cmdIFFT_vertical.images[2].texture = cmdIFFT_horizontal.images[2].texture;

  for (int i = 0; i < cfg.logSize; i++) {
    shaderIFFT_vertical->setUniform1i("u_step", i);
    renderer->submit(cmdIFFT_vertical);
    renderer->dispatch();
    renderer->memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    std::swap(cmdIFFT_vertical.images[1], cmdIFFT_vertical.images[2]);
  }

  cmdPermute.images[0].texture = cmdIFFT_vertical.images[1].texture;
  renderer->submit(cmdPermute);
  renderer->dispatch();
  renderer->memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Tessendorf::generateMerge(float dt) {
  auto _task = profiler->startScopedTaskGpu(querieMerge);

  shaderMerge->setUniform1f("u_lambda", cfg.lambda);
  shaderMerge->setUniform1f("u_dt", dt);
  renderer->submit(cmdMerge);
  renderer->dispatch();
  renderer->memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

} // namespace gfx::terrain::water

