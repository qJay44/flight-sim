#pragma once

#include <stack>

#include "../engine/texture/Texture2DArray.hpp"
#include "../engine/mesh/BufferObject.hpp"
#include "NodeData.hpp"

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

  ~GenerationManager() = default;

  void update();

  [[nodiscard]] int acquireSlot();
  void freeSlot(int slot);
  void freeSlotAll();

  void generate(const NodeData& node);
  void bindTexture(GLuint slot = 0) const;

private:
  friend ::gui;

  Texture2DArray texArray;
  GLuint numGroups = 0;
  std::stack<int> freeSlots;

  struct Config {
    float continentFreq = 1.2f;
    float initAmplitude = 0.5f;
    float initFrequency = 1.0f;
    float gain = 0.5f;
    float lacunarity = 2.0f;
    float canyonSteps = 15.f;
    float fbmOffsetFreq1 = 3.f;
    float fbmOffsetFreq2 = 3.f;
    float fbmOffsetFreq3 = 3.f;
    float fbmOffsetOffset2 = 12.5f;
    float fbmOffsetOffset3 = 45.1f;
    float fbmOffsetTwist = 0.25f;
    float f1VoronoiFreq1 = 5.f;
    float f1VoronoiFreq2 = 10.5f;
    float detailInitAmplitude = 0.5f;
    float detailInitFrequency = 12.0f;
    float detailGain = 0.48f;
    float detailLacunarity = 2.1f;
    int octaves = 2;
    int detailOctaves = 10;
    float _pad[0];
  } cfg;
  static_assert(sizeof(Config) % 16 == 0);

  struct {
    BufferObject config{GL_UNIFORM_BUFFER, false};
  } ubo;
};

} // terrain

