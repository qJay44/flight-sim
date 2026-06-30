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
    float initAmplitude = 0.5f;
    float initFrequency = 1.0f;
    float gain = 0.5f;
    float lacunarity = 2.0f;
    float amplitudeMult = 0.05f;
    float frequencyMult = 45.f;
    float detailInitAmplitude = 0.5f;
    float detailInitFrequency = 12.0f;
    float detailGain = 0.48f;
    float detailLacunarity = 2.1f;
    float detailAmplitudeMult = 0.05f;
    float detailFrequencyMult = 45.f;
    int octaves = 3;
    int detailOctaves = 10;
    vec2 _pad;
  } cfg;
  static_assert(sizeof(Config) % 16 == 0);

  struct {
    BufferObject config{GL_UNIFORM_BUFFER, false};
  } ubo;
};

} // terrain

