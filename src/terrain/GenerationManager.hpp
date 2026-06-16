#pragma once

#include "../engine/texture/Texture2D.hpp"
#include "../engine/Shader.hpp"
#include "../engine/mesh/BufferObject.hpp"

struct gui;

namespace terrain {

class GenerationManager {
public:
  enum Status {
    IDLE = 0,
    GENERATING = 1,
    DONE = 2
  };

  GenerationManager() = default;
  GenerationManager(int resolution);

  const float& getHeightmapScale() const;

  void setHeightmapScale(float s);

  void generateInit();
  void generate(ivec2 pixelDelta, vec2 offset);

  Status checkStatus();
  void bindTextures(GLuint slotA = 0, GLuint slotB = 1) const;

private:
  friend ::gui;

  static Shader shaderBufferA;
  static Shader shaderBufferB;

  struct alignas(16) ErosionConfig {
    float erosion_scale = 0.15;
    float erosion_strength = 0.22;
    float erosion_gully_weight = 0.5;
    float erosion_detail = 1.5;

    float ridgeRounding = 0.1;
    float creaseRounding = 0.0;
    float _pad0[2];

    vec4 erosion_rounding = vec4(ridgeRounding, creaseRounding, 0.1, 2.0);
    vec4 erosion_onset = vec4(1.25, 1.25, 2.8, 1.5);
    vec2 erosion_assumed_slope = vec2(0.7, 1.0);

    float erosion_cell_scale = 0.7;
    float erosion_normalization = 0.5;
    int erosion_octaves = 5;
    float erosion_lacunarity = 2.0;
    float erosion_gain = 0.5;

    float _pad1;

    vec2 terrain_height_offset = vec2(-0.65, 0.0);

    float height_frequency = 3.0;
    float height_amp = 0.125;
    int height_octaves = 3;
    float height_lacunarity = 2.0;
    float height_gain = 0.1;

    float heightFunctionScale = 1.0;
  };
  static_assert(sizeof(ErosionConfig) % 16 == 0);

  struct Task {
    ivec2 dirtyX;
    ivec2 dirtyY;
    ivec2 pixelDelta;
    vec2 offset;
  };

  struct {
    BufferObject erosionConfig{GL_UNIFORM_BUFFER};
  } ubo;

  int resolution;

  // Index = 0: Pair (A0, B0)
  // Index = 1: Pair (A1, B1)
  Texture2D buffers[2][2];
  int currReadIdx = 0;

  GLsync fence = nullptr;
  Status status = IDLE;

  Task activeTask{};
  int currOffsetY = 0;
  int sliceHeight = 256;

  ivec2 texWriteHead{};
  float heightmapScale = 1.f;

  ErosionConfig erosionConfig{};

private:
  void updateBufferA();
  void updateBufferB();
};

} // terrain

