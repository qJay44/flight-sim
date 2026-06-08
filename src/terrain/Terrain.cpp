#include "Terrain.hpp"

#include "../engine/mesh/meshes.hpp"
#include "Terrain.hpp"
#include "TextureManager.hpp"
#include "colormaps/jet.hpp"
#include "glm/common.hpp"
#include "glm/geometric.hpp"

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

  chunkSize = 128.f;
  chunkSizeInv = 1.f / chunkSize;

  mesh0.setMatScaleXZ(chunkSize * 0.5f);
  mesh1.setMatScaleXZ(chunkSize * 0.5f);
  mesh2.setMatScaleXZ(chunkSize * 0.5f);
}

const float& Terrain::getHeightScale() const { return heightScale; }

void Terrain::update(const Camera* cam) {
  const vec3& camPos = cam->getPosition();
  vec2 pos = vec2{camPos.x, camPos.z};
  ivec2 cameraCoord = glm::floor(pos * chunkSizeInv);

  if (cameraCoord == lastCoord)
    return;

  evictDistantChunks(cameraCoord, radius);

  visibleChunks.clear();

  for (int z = -radius; z <= radius; z++) {
    for (int x = -radius; x <= radius; x++) {
      ivec2 targetCoord{cameraCoord + ivec2(x, z)};
      auto [it, inserted] = chunksCache.emplace(targetCoord, Chunk{});

      if (inserted) {
        Chunk& newChunk = it->second;
        vec2 targetCoordf = targetCoord;
        newChunk.worldPos = targetCoordf * chunkSize;
        newChunk.textureSlot = texManager.acquireSlot(targetCoordf);
      }

      visibleChunks.push_back(&it->second);
    }
  }

  lastCoord = cameraCoord;
}

void Terrain::draw(const Camera* cam, Shader& shader, const Environment& env) const {
  shader.setUniform1f("u_heightScale", heightScale);
  shader.setUniform1f("u_chunkSize", chunkSize);
  shader.setUniform1f("u_showChunkGroups", showChunkGroups);
  shader.setUniform1i("u_bufferSize", bufferSize);

  texManager.bind();
  env.skybox.bind(2);
  ssbo.chunks.bindBase(0);

  std::vector<Chunk> c0;
  std::vector<Chunk> c1;
  std::vector<Chunk> c2;

  const vec3& camPos = cam->getPosition();
  const float& camFar = cam->getFarPlane();
  vec2 camPosXZ{camPos.x, camPos.z};

  for (const Chunk* chunk : visibleChunks) {
    float dist = glm::distance(camPosXZ, chunk->worldPos);

    if      (dist < camFar * 0.05f) c0.push_back(*chunk);
    else if (dist < camFar * 0.1f)  c1.push_back(*chunk);
    else                            c2.push_back(*chunk);
  }

  if (!c0.empty()) {
    shader.setUniform3f("u_debugChunkGroupColor", colormaps::jet[0]);
    ssbo.chunks.allocate(c0.data(), c0.size() * sizeof(Chunk), GL_DYNAMIC_DRAW);
    mesh0.draw(cam, shader, c0.size());
  }

  if (!c1.empty()) {
    shader.setUniform3f("u_debugChunkGroupColor", colormaps::jet[6]);
    ssbo.chunks.allocate(c1.data(), c1.size() * sizeof(Chunk), GL_DYNAMIC_DRAW);
    mesh1.draw(cam, shader, c1.size());
  }

  if (!c2.empty()) {
    shader.setUniform3f("u_debugChunkGroupColor", colormaps::jet[9]);
    ssbo.chunks.allocate(c2.data(), c2.size() * sizeof(Chunk), GL_DYNAMIC_DRAW);
    mesh2.draw(cam, shader, c2.size());
  }
}

int Terrain::getTotalChunksFromRadius(int radius) {
  assert(radius);

  int res = 9;

  for (int i = 0; i < radius - 1; i++)
    res += (3 + (i + 1) * 2) * 4 - 4;

  return res;
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
    texManager.releaseSlot(chunksCache[key].textureSlot);
    chunksCache.erase(key);
  }
}

} // terrain

