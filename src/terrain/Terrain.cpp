#include "Terrain.hpp"

#include "../engine/mesh/meshes.hpp"
#include "Terrain.hpp"
#include "TextureManager.hpp"
#include "colormaps/jet.hpp"
#include "glm/common.hpp"
#include "glm/geometric.hpp"
#include "utils/utils.hpp"
#include <utility>

#define CHUNKS_PER_FRAME 1

namespace terrain {

Terrain::Terrain(int bufferSize, int radius)
  : bufferSize(bufferSize),
    radius(radius),
    bufferSizeInv(1.f / vec2(bufferSize))
{
  int totalChunks = getTotalChunksFromRadius(radius);
  texManager = TextureManager(totalChunks, ivec2(bufferSize));

  mesh0 = meshes::plane(128);
  mesh1 = meshes::plane(64);
  mesh2 = meshes::plane(32);

  GLbitfield flags =
    GL_MAP_READ_BIT       |
    GL_MAP_WRITE_BIT      |
    GL_MAP_PERSISTENT_BIT |
    GL_MAP_COHERENT_BIT;

  ssbo.chunks.storage(nullptr, totalChunks * sizeof(Chunk), flags);

  mappedChunks = (Chunk*)ssbo.chunks.mapRange(totalChunks * sizeof(Chunk), flags);

  for (int i = 0; i < totalChunks; i++)
    freeChunks.push(i);

  ubo.erosionConfig.allocate(&erosionConfig, sizeof(ErosionConfig), GL_DYNAMIC_DRAW);

  changeScale(128.f);
}

const float& Terrain::getHeightScale() const { return heightScale; }

void Terrain::update(const Camera* cam) {
  const vec3& camPos = cam->getPosition();
  vec2 posXZ{camPos.x, camPos.z};
  ivec2 cameraCoord = glm::floor(posXZ * chunkSizeInv);

  if (cameraCoord != lastCoord || forceUpate) {
    forceUpate = false;

    evictDistantChunks(cameraCoord, radius);

    ssbo.chunks.bindBase(0);
    ubo.erosionConfig.bindBase(0);

    bool skip = false;

    for (int z = -radius; z <= radius && !skip; z++) {
      for (int x = -radius; x <= radius; x++) {
        // If moving too fast a lot of chunks may be processed, so skip for now
        if (freeChunks.empty()) {
          skip = true;
          break;
        }

        ivec2 targetCoord{cameraCoord + ivec2(x, z)};
        auto [it, inserted] = chunksCache.emplace(targetCoord, freeChunks.top());
        Chunk& chunk = mappedChunks[it->second];

        if (inserted) {
          freeChunks.pop();
          chunk.worldPos = vec2(targetCoord) * chunkSize;
          chunk.index = it->second;
          chunk.state = Chunk::GENERATING;
          texManager.generate(targetCoord, it->second);
          chunk.syncFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        }
      }
    }

    lastCoord = cameraCoord;
  }

  const float& camFar = cam->getFarPlane();

  c0.clear();
  c1.clear();
  c2.clear();

  for (auto const& [coord, index] : chunksCache) {
    Chunk& chunk = mappedChunks[index];

    switch (chunk.state) {
      case Chunk::PENDING:
        error("[Terrain::update] This should never happen");
        break;
      case Chunk::GENERATING:
        if (chunk.syncFence) {
          GLenum status = glClientWaitSync(chunk.syncFence, 0, 0);

          if (status == GL_ALREADY_SIGNALED || status == GL_CONDITION_SATISFIED) {
            glDeleteSync(chunk.syncFence);
            chunk.syncFence = nullptr;
            chunk.state = Chunk::ACTIVE;
            pushToDraw(chunk, posXZ, camFar);
          }
        }
        break;
      case Chunk::ACTIVE:
        pushToDraw(chunk, posXZ, camFar);
        break;
    }
  }

  MeshData sharedInstanceData{};
  sharedInstanceData.usage = GL_STREAM_DRAW;
  sharedInstanceData.layout = ChunkIndexInstance::getLayout();

  sharedInstanceData.vertices = c0.data();
  sharedInstanceData.verticesSize = c0.size() * sizeof(ChunkIndexInstance);
  mesh0.setInstanceVBO(sharedInstanceData);

  sharedInstanceData.vertices = c1.data();
  sharedInstanceData.verticesSize = c1.size() * sizeof(ChunkIndexInstance);
  mesh1.setInstanceVBO(sharedInstanceData);

  sharedInstanceData.vertices = c2.data();
  sharedInstanceData.verticesSize = c2.size() * sizeof(ChunkIndexInstance);
  mesh2.setInstanceVBO(sharedInstanceData);
}

void Terrain::regenerateAllChunks() {
  int totalChunks = getTotalChunksFromRadius(radius);

  chunksCache.clear();

  while (!freeChunks.empty())
    freeChunks.pop();

  for (int i = 0; i < totalChunks; i++) {
    freeChunks.push(i);
    invalidateChunk(i);
  }

  ubo.erosionConfig.allocate(&erosionConfig, sizeof(ErosionConfig), GL_DYNAMIC_DRAW);

  forceUpate = true;
}

void Terrain::changeScale(float s) {
  chunkSize = s;
  chunkSizeInv = 1.f / chunkSize;

  mesh0.setMatScaleXZ(chunkSize * 0.5f);
  mesh1.setMatScaleXZ(chunkSize * 0.5f);
  mesh2.setMatScaleXZ(chunkSize * 0.5f);

  regenerateAllChunks();
}

void Terrain::draw(const Camera* cam, Shader& shader) const {
  shader.setUniform1f("u_heightScale", heightScale);
  shader.setUniform1f("u_chunkSize", chunkSize);
  shader.setUniform1f("u_showChunkGroups", showChunkGroups);
  shader.setUniform1i("u_bufferSize", bufferSize);

  texManager.bind();
  ssbo.chunks.bindBase(0);

  if (!c0.empty()) {
    shader.setUniform3f("u_debugChunkGroupColor", colormaps::jet[0]);
    mesh0.draw(cam, shader);
  }

  if (!c1.empty()) {
    shader.setUniform3f("u_debugChunkGroupColor", colormaps::jet[6]);
    mesh1.draw(cam, shader);
  }

  if (!c2.empty()) {
    shader.setUniform3f("u_debugChunkGroupColor", colormaps::jet[9]);
    mesh2.draw(cam, shader);
  }
}

int Terrain::getTotalChunksFromRadius(int radius) {
  assert(radius);

  constexpr int extra = 10; // Calculate dynamically?

  int res = 9;

  for (int i = 0; i < radius - 1; i++)
    res += (3 + (i + 1) * 2) * 4 - 4;

  return res + extra;
}

void Terrain::invalidateChunk(int idx) {
  Chunk& chunk = mappedChunks[idx];
  chunk.state = Chunk::PENDING;

  if (chunk.syncFence)
    glDeleteSync(std::exchange(chunk.syncFence, nullptr));
}

void Terrain::evictDistantChunks(ivec2 currChunkCoord, int maxRadius) {
  int evictRadius = maxRadius;
  std::vector<ivec2> keysToRemove;

  // Do not mix with chunksCache.erase()
  for (const auto& [coord, chunk] : chunksCache) {
    ivec2 delta = glm::abs(coord - currChunkCoord);

    if (delta.x > evictRadius || delta.y > evictRadius)
      keysToRemove.push_back(coord);
  }

  for (const auto& key : keysToRemove) {
    int idx = chunksCache[key];
    freeChunks.push(idx);
    chunksCache.erase(key);

    invalidateChunk(idx);
  }
}

void Terrain::pushToDraw(const Chunk& chunk, vec2 camPos, float camFar) {
  float dist = glm::distance(camPos, chunk.worldPos);

  if      (dist < camFar * 0.05f) c0.push_back(chunk.index);
  else if (dist < camFar * 0.1f)  c1.push_back(chunk.index);
  else                            c2.push_back(chunk.index);
}

} // terrain

