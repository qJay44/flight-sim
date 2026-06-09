#pragma once

#include <functional>
#include <unordered_map>

#include "../engine/Camera.hpp"
#include "../engine/mesh/MeshElementsInstancing.hpp"
#include "TextureManager.hpp"
#include "../Environment.hpp"

struct gui;

namespace terrain {

class Terrain {
public:
  Terrain(int bufferSize, int resolution);

  const float& getHeightScale() const;

  void update(const Camera* cam);
  void draw(const Camera* cam, Shader& shader, const Environment& env) const;

private:
  friend struct ::gui;

  // alignans(8) becuase the largest element in std430 is a vec2 (8 bytes)
  struct alignas(8) Chunk {
    vec2 worldPos;
    int textureSlot;
  };

  struct ChunkHash {
    std::size_t operator() (const ivec2& v) const {
      size_t h1 = std::hash<int>{}(v.x);
      size_t h2 = std::hash<int>{}(v.y);
      return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
  };

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

  struct {
    BufferObject chunks{GL_SHADER_STORAGE_BUFFER};
  } ssbo;

  struct {
    BufferObject erosionConfig{GL_UNIFORM_BUFFER};
  } ubo;

  int bufferSize;
  int radius;

  vec2 bufferSizeInv;
  TextureManager texManager;
  ErosionConfig erosionConfig{};

  MeshElementsInstancing mesh0; // 3x3
  MeshElementsInstancing mesh1; // 5x5 (ring)
  MeshElementsInstancing mesh2; // 7x7 (ring)

  float chunkSize{1.f};
  float chunkSizeInv{1.f};
  ivec2 lastCoord{9999};
  std::unordered_map<ivec2, Chunk, ChunkHash> chunksCache;
  std::vector<Chunk*> visibleChunks;

  float heightScale = 30.f;

  bool showChunkGroups = false;

private:
  static int getTotalChunksFromRadius(int radius);

  void evictDistantChunks(ivec2 currChunk, int maxRadius);
};

} // terrain

