#include "Terrain.hpp"

#include "../engine/mesh/meshes.hpp"
#include "GenerationManager.hpp"
#include "Terrain.hpp"
#include "glm/common.hpp"

namespace terrain {

Terrain::Terrain(int bufferSize, int rings) : bufferSize(bufferSize), rings(rings) {
  texManager = GenerationManager(bufferSize);
  texManager.setHeightmapScale(5.f);
  texManager.generateInit();

  heightScale = 300.f;

  meshCore = meshes::plane(128);
  meshCore.setInstanceCount(16);

  meshRing = meshes::plane(128);

  std::vector<DrawElementsIndirectCommand> cmds(rings);
  for (int i = 0; i < rings; i++) {
    auto& cmd = cmds[i];
    cmd.count = meshRing.getElementCount();
    cmd.instanceCount = 12;
    cmd.baseInstance = 12 * i;
  }

  ibo.cmd.storage(cmds.data(), cmds.size() * sizeof(DrawElementsIndirectCommand), 0);
}

void Terrain::update(const Camera* cam) {
  const vec3& camPos = cam->getPosition();
  vec2 snap = glm::floor(vec2{camPos.x, camPos.z} / chunkSize) * chunkSize;
  vec2 movementDelta = snap - gridAnchor;

  using enum GenerationManager::Status;
  texManager.checkStatus();

  if (glm::length2(movementDelta) >= chunkSize * chunkSize) {
    float totalWorldWidth = 4.f * glm::exp2(rings) * chunkSize;
    globalOffsetUV -= movementDelta / totalWorldWidth;
    gridAnchor = snap;

    int pixelsPerChunk = bufferSize / 16;
    ivec2 chunkDelta = ivec2(glm::round(movementDelta / chunkSize));
    ivec2 pixelDelta = chunkDelta * pixelsPerChunk;

    texManager.generate(pixelDelta, pending.globalOffsetUV);
  }
}

void Terrain::drawCore(const Camera* cam, Shader& shader) const {
  setTerrainUniforms(shader);
  texManager.bindTextures();

  meshCore.draw(cam, shader);
}

void Terrain::drawCoreShadow(const Camera* cam, Shader& shader) const {
  setTerrainUniforms(shader);
  texManager.bindTextures();

  meshCore.draw(cam, shader);
}

void Terrain::drawRings(const Camera* cam, Shader& shader) const {
  setTerrainUniforms(shader);
  texManager.bindTextures();

  ibo.cmd.bindBaseAs(GL_SHADER_STORAGE_BUFFER, 0);
  meshRing.drawMultiIndirect(cam, shader, ibo.cmd, rings);
}

void Terrain::drawRingsShadow(const Camera* cam, Shader& shader) const {
  setTerrainUniforms(shader);
  texManager.bindTextures();

  ibo.cmd.bindBaseAs(GL_SHADER_STORAGE_BUFFER, 0);
  meshRing.drawMultiIndirect(cam, shader, ibo.cmd, rings);
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

void Terrain::setTerrainUniforms(Shader& shader) const {
  shader.setUniform1f("u_chunkSize", chunkSize);
  shader.setUniform1f("u_heightScale", heightScale);
  shader.setUniform1f("u_heightmapScale", texManager.getHeightmapScale());
  shader.setUniform1i("u_rings", rings);
  shader.setUniform1i("u_debugLOD", debugLOD);
  shader.setUniform2f("u_globalOffsetUV", globalOffsetUV);
  shader.setUniform2f("u_gridAnchor", gridAnchor);
  shader.setUniform2f("u_cliffEdges", cliffEdges);
  shader.setUniform2f("u_dirtEdges", dirtEdges);
  shader.setUniform2f("u_snowEdges", snowEdges);
  shader.setUniform2f("u_sandEdges", sandEdges);
  shader.setUniform2f("u_grass0Edges", grass0Edges);
  shader.setUniform2f("u_grass1Edges", grass1Edges);
  shader.setUniform2f("u_grass2Edges", grass2Edges);
}

} // terrain

