#include "Tessendorf.hpp"

#include "glm/common.hpp"
#include "glm/exponential.hpp"
#include "global.hpp"
#include "utils/utils.hpp"
#include <cassert>
#include <utility>

namespace terrain::water {

bool Tessendorf::isInitialized = false;

Tessendorf::Tessendorf() {
  if (std::exchange(isInitialized, true))
    error("[Tessendorf::Tessendorf] Tessendorf instance already exists");

  ubo.spectrums.gen();
  ubo.spectrums.storage(nullptr, sizeof(spectrums), GL_DYNAMIC_STORAGE_BIT);
  build();
}

void Tessendorf::updateInitials() {
  generateButterfly();
  generateNoise();
  generateInitialSpectrum();
}

void Tessendorf::update() {
  if (std::exchange(rebuild, false))
    build();

  generateWavesAtTime(global::time);
  generateIFFT(texDxDz, texBuffer);
  generateIFFT(texDyDxz, texBuffer);
  generateIFFT(texDyxDyz, texBuffer);
  generateIFFT(texDxxDzz, texBuffer);
  generateMerge();
}

void Tessendorf::markForRebuild() {
  rebuild = true;
}

void Tessendorf::bindTextures(GLuint displacementUnit, GLuint derivativesUnit, GLuint turbulenceUnit) const {
  texDisplacement.bind(displacementUnit);
  texDerivatives.bind(derivativesUnit);
  texTurbulence.bind(turbulenceUnit);
}

void Tessendorf::build() {
  TextureDescriptor descR    { .internalFormat = GL_R32F,    .format = GL_RED,  .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
  TextureDescriptor descRG   { .internalFormat = GL_RG32F,   .format = GL_RG,   .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
  TextureDescriptor descRGBA { .internalFormat = GL_RGBA32F, .format = GL_RGBA, .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
  logSize = glm::log2((float)size);
  numWorkGroups = size / 8;

  texButterfly       = Texture2D(ivec2(logSize, size), descRGBA);
  texNoise           = Texture2D(size, descRG);
  texInitialSpectrum = Texture2D(size, descRGBA);
  texPrecomputedData = Texture2D(size, descRGBA);
  texBuffer          = Texture2D(size, descRG);
  texDxDz            = Texture2D(size, descRG);
  texDyDxz           = Texture2D(size, descRG);
  texDyxDyz          = Texture2D(size, descRG);
  texDxxDzz          = Texture2D(size, descRG);
  texDisplacement    = Texture2D(size, descRGBA);
  texDerivatives     = Texture2D(size, descRGBA);
  texTurbulence      = Texture2D(size, descR);

  updateInitials();
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
  settings.alpha = JonswapAlpha(g, display.fetch, display.windSpeed);
  settings.peakOmega = JonswapPeakFrequency(g, display.fetch, display.windSpeed);
  settings.gamma = display.peakEnhancemnt;
  settings.shortWavesFade = display.shortWavesFade;
}

void Tessendorf::generateButterfly() {
  shaderButterfly.use();
  shaderButterfly.setUniform1i("u_size", size);
  glBindImageTexture(0, texButterfly.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glDispatchCompute(logSize, numWorkGroups / 2, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Tessendorf::generateNoise() {
  shaderNoise.use();
  shaderNoise.setUniform1f("u_seed1", seed1);
  shaderNoise.setUniform1f("u_seed2", seed2);
  glBindImageTexture(0, texNoise.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
  glDispatchCompute(numWorkGroups, numWorkGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Tessendorf::generateInitialSpectrum() {
  fillSettings(local, spectrums[0]);
  fillSettings(swell, spectrums[1]);
  ubo.spectrums.updateSubData(spectrums, sizeof(spectrums));

  ubo.spectrums.bindBase(0);
  texNoise.bind(0);

  shaderInitialSpectrum.use();
  shaderInitialSpectrum.setUniform1f("u_g", g);
  shaderInitialSpectrum.setUniform1f("u_depth", depth);
  shaderInitialSpectrum.setUniform1f("u_lengthScale", lengthScale);
  glBindImageTexture(0, texNoise.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
  glBindImageTexture(1, texBuffer.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
  glBindImageTexture(2, texPrecomputedData.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glDispatchCompute(numWorkGroups, numWorkGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  shaderConjugateSpectrum.use();
  glBindImageTexture(0, texBuffer.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
  glBindImageTexture(1, texInitialSpectrum.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glDispatchCompute(numWorkGroups, numWorkGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Tessendorf::generateWavesAtTime(float time) {
  global::profiler.startScopedTaskGpu(querieTimeEvolution);

  shaderTimeSpectrum.use();
  shaderTimeSpectrum.setUniform1f("u_time", time);
  glBindImageTexture(0, texInitialSpectrum.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
  glBindImageTexture(1, texPrecomputedData.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
  glBindImageTexture(2, texDxDz  .getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
  glBindImageTexture(3, texDyDxz .getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
  glBindImageTexture(4, texDyxDyz.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
  glBindImageTexture(5, texDxxDzz.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
  glDispatchCompute(numWorkGroups, numWorkGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Tessendorf::generateIFFT(Texture2D& input, Texture2D& buffer) {
  global::profiler.startScopedTaskGpu(querieIFFT);

  GLuint ping = input.getId();
  GLuint pong = buffer.getId();
  glBindImageTexture(0, texButterfly.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

  shaderIFFT_horizontal.use();
  for (int i = 0; i < logSize; i++) {
    shaderIFFT_horizontal.setUniform1i("u_step", i);
    glBindImageTexture(1, ping, 0, GL_FALSE, 0, GL_READ_ONLY , GL_RG32F);
    glBindImageTexture(2, pong, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
    glDispatchCompute(numWorkGroups, numWorkGroups, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    std::swap(ping, pong);
  }

  shaderIFFT_vertical.use();
  for (int i = 0; i < logSize; i++) {
    shaderIFFT_vertical.setUniform1i("u_step", i);
    glBindImageTexture(1, ping, 0, GL_FALSE, 0, GL_READ_ONLY , GL_RG32F);
    glBindImageTexture(2, pong, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
    glDispatchCompute(numWorkGroups, numWorkGroups, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    std::swap(ping, pong);
  }

  shaderPermute.use();
  glBindImageTexture(0, input.getId(), 0, GL_FALSE, 0, GL_READ_WRITE , GL_RG32F);
  glDispatchCompute(numWorkGroups, numWorkGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Tessendorf::generateMerge() {
  global::profiler.startScopedTaskGpu(querieMerge);

  shaderMerge.use();
  shaderMerge.setUniform1f("u_lambda", lambda);
  shaderMerge.setUniform1f("u_dt", global::dt);
  glBindImageTexture(0, texDxDz  .getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
  glBindImageTexture(1, texDyDxz .getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
  glBindImageTexture(2, texDyxDyz.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
  glBindImageTexture(3, texDxxDzz.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
  glBindImageTexture(4, texDisplacement.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY , GL_RGBA32F);
  glBindImageTexture(5, texDerivatives.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY , GL_RGBA32F);
  glBindImageTexture(6, texTurbulence.getId(), 0, GL_FALSE, 0, GL_READ_WRITE , GL_R32F);
  glDispatchCompute(numWorkGroups, numWorkGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

} // namespace water


