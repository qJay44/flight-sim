#include "Terrain.hpp"

#include "../engine/mesh/meshes.hpp"
#include "Terrain.hpp"
#include "glm/common.hpp"
#include "glm/geometric.hpp"
#include "global.hpp"
#include "utils/utils.hpp"

namespace terrain {

Terrain::Terrain(int bufferSize, int radius)
  : bufferSize(bufferSize),
    radius(radius),
    bufferSizeInv(1.f / vec2(bufferSize))
{
  int totalChunks = getTotalChunksFromRadius(radius);
  texManager = GenerationManager(totalChunks, ivec2(bufferSize));
  texManager.setHeightmapScale(0.15f);
  heightScale = 64.f;

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

  changeScale(32.f);
}

const float& Terrain::getHeightScale() const { return heightScale; }

void Terrain::update(const Camera* cam) {
  const vec3& camPos = cam->getPosition();
  vec2 posXZ{camPos.x, camPos.z};
  ivec2 cameraCoord = glm::floor(posXZ * chunkSizeInv);

  if (cameraCoord != lastCoord || forceUpate) {
    global::profiler->startScopedTask("NewCoordChunks");
    forceUpate = false;

    evictDistantChunks(cameraCoord, radius);

    global::profiler->startScopedTask("ChunksNewCoordPass");
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

  global::profiler->startScopedTask("ChunksSelectionPass");

  const float& camFar = cam->getFarPlane();

  c0.clear();
  c1.clear();
  c2.clear();

  global::profiler->startScopedTask("ChunksPush");
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

  appearance = glm::min(appearance + 1.5f * global::dt, 1.f);
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
  appearance = 0.f;
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
  shader.setUniform1f("u_heightmapScale", texManager.getHeightmapScale());
  shader.setUniform1f("u_chunkSize", chunkSize);
  shader.setUniform1f("u_showChunkGroups", showChunkGroups);
  shader.setUniform1f("u_appearance", appearance);
  shader.setUniform1i("u_bufferSize", bufferSize);
  shader.setUniform2f("u_cliffEdges", cliffEdges);
  shader.setUniform2f("u_dirtEdges", dirtEdges);
  shader.setUniform2f("u_snowEdges", snowEdges);
  shader.setUniform2f("u_sandEdges", sandEdges);
  shader.setUniform2f("u_grass0Edges", grass0Edges);
  shader.setUniform2f("u_grass1Edges", grass1Edges);
  shader.setUniform2f("u_grass2Edges", grass2Edges);

  texManager.bindTextures();
  ssbo.chunks.bindBase(0);

  if (!c0.empty()) {
    shader.setUniform3f("u_debugChunkGroupColor", vec3(1.f, 0.1f, 0.1f));
    mesh0.draw(cam, shader);
  }

  if (!c1.empty()) {
    shader.setUniform3f("u_debugChunkGroupColor", vec3(0.1f, 1.f, 0.1f));
    mesh1.draw(cam, shader);
  }

  if (!c2.empty()) {
    shader.setUniform3f("u_debugChunkGroupColor", vec3(0.1f, 0.1f, 1.f));
    mesh2.draw(cam, shader);
  }
}

void Terrain::drawPostprocess(const Camera* cam, Shader& shader) const {
  const mat4& camProj = cam->getProj();
  const mat4& localView = cam->getLocalView(vec3(0.f));

  shader.setUniform1f("u_heightScale", heightScale);
  shader.setUniform1f("u_waterShoreScale", waterShoreScale);
  shader.setUniform1f("u_waterRefractionScale", waterRefractionScale);
  shader.setUniform1f("u_waterRefractionDistortScale", waterRefractionDistortScale);
  shader.setUniform1f("u_foamEdge0", foamEdge0);
  shader.setUniform1f("u_foamEdge1", foamEdge1);
  shader.setUniform1f("u_waterNormalScaleUV", waterNormalScaleUV);
  shader.setUniform1f("u_waterNoiseScale", waterNoiseScale);
  shader.setUniform1f("u_fogDensity", fogDensity);
  shader.setUniform1f("u_fogDensityFalloff", fogDensityFalloff);
  shader.setUniform1f("u_horizonThickness", horizonThickness);
  shader.setUniform1f("u_horizonFalloff", horizonFalloff);
  shader.setUniformMatrix4f("u_invPV", glm::inverse(camProj * localView));

  Mesh::drawScreen(cam, shader);
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

  if      (dist < camFar * 0.2f)  c0.push_back(chunk.index);
  else if (dist < camFar * 0.6f)  c1.push_back(chunk.index);
  else                            c2.push_back(chunk.index);
}

} // terrain

