#pragma once

#include <functional>
#include <unordered_map>

#include "../engine/Camera.hpp"
#include "../engine/mesh/MeshElementsInstancing.hpp"
#include "TextureManager.hpp"

struct gui;

namespace terrain {

class Terrain {
public:
  Terrain(int bufferSize, int resolution);

  void update(const Camera* cam);
  void draw(const Camera* cam, Shader& shader) const;

private:
  friend struct ::gui;

  #pragma pack(push, 1)
  struct Chunk {
    vec2 worldPos;
    int textureSlot;
    int padding;
  };
  #pragma pack(pop)

  struct ChunkHash {
    std::size_t operator() (const ivec2& v) const {
      size_t h1 = std::hash<int>{}(v.x);
      size_t h2 = std::hash<int>{}(v.y);
      return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
  };

  struct {
    BufferObject chunks{GL_SHADER_STORAGE_BUFFER};
  } ssbo;

  int bufferSize;
  int radius;

  vec2 bufferSizeInv;
  TextureManager texManager;

  MeshElementsInstancing mesh0; // 3x3
  MeshElementsInstancing mesh1; // 5x5 (ring)
  MeshElementsInstancing mesh2; // 7x7 (ring)

  float chunkSize{1.f};
  float chunkSizeInv{1.f};
  ivec2 lastCoord{9999};
  std::unordered_map<ivec2, Chunk, ChunkHash> chunksCache;
  std::vector<Chunk*> visibleChunks;

  float heightScale = 0.f;

  bool showChunkGroups = false;

private:
  static int getTotalChunksFromRadius(int radius);

  void evictDistantChunks(ivec2 currChunk, int maxRadius);
};

} // terrain

