#pragma once

#include <functional>
#include <stack>
#include <unordered_map>

#include "../engine/Camera.hpp"
#include "../engine/mesh/MeshElementsInstancing.hpp"
#include "GenerationManager.hpp"

struct gui;

namespace terrain {

class Terrain {
public:
  Terrain(int bufferSize, int resolution);

  const float& getHeightScale() const;

  void update(const Camera* cam);
  void regenerateAllChunks();
  void changeScale(float s);
  void draw(const Camera* cam, Shader& shader) const;

private:
  friend struct ::gui;

  // alignans(8) becuase the largest element in std430 is a vec2 (8 bytes)
  struct alignas(8) Chunk {
    enum State {
      PENDING    = 0,
      GENERATING = 1,
      ACTIVE     = 2,
    };
    vec2 worldPos;
    int index;
    int state = PENDING;

    GLsync syncFence = nullptr;
  };

  struct ChunkHash {
    std::size_t operator() (const ivec2& v) const {
      size_t h1 = std::hash<int>{}(v.x);
      size_t h2 = std::hash<int>{}(v.y);
      return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
  };

  struct ChunkIndexInstance {
    int index;

    static const vertex::Layout& getLayout() {
      static constexpr vertex::Attribute attribs[] = {
        {2, 1, GL_INT}
      };

      static constexpr vertex::Layout layout = {attribs, 1, sizeof(ChunkIndexInstance)};

      return layout;
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
  GenerationManager texManager;
  ErosionConfig erosionConfig{};

  MeshElementsInstancing mesh0; // 3x3
  MeshElementsInstancing mesh1; // 5x5 (ring)
  MeshElementsInstancing mesh2; // 7x7 (ring)

  float chunkSize{1.f};
  float chunkSizeInv{1.f};
  ivec2 lastCoord{9999};
  std::unordered_map<ivec2, int, ChunkHash> chunksCache;
  std::vector<Chunk*> visibleChunks;

  Chunk* mappedChunks = nullptr;
  std::stack<int> freeChunks;

  std::vector<GLuint> c0;
  std::vector<GLuint> c1;
  std::vector<GLuint> c2;

  float heightScale = 0.5f;
  float appearance = 0.f;
  vec2 cliffEdges  {0.40f , 0.52f};
  vec2 dirtEdges   {0.60f , 0.00f}; // inversed
  vec2 snowEdges   {0.53f , 0.60f};
  vec2 sandEdges   {0.005f, 0.00f}; // inversed, WATER_HEIGHT offset
  vec2 grass0Edges {0.40f , 0.60f};
  vec2 grass1Edges {0.05f , 0.02f}; // inversed, GRASS_HEIGHT offset
  vec2 grass2Edges {0.80f , 0.10f};

  bool showChunkGroups = false;
  bool forceUpate = false;

private:
  static int getTotalChunksFromRadius(int radius);

  void invalidateChunk(int idx);
  void evictDistantChunks(ivec2 currChunk, int maxRadius);
  void pushToDraw(const Chunk& chunk, vec2 camPos, float camFar);
};

} // terrain

