#include "Terrain.hpp"

#include <cassert>

#include "glm/ext/matrix_transform.hpp"
#include "shared.hpp"
#include "quadtree.hpp"

namespace terrain {

Terrain::Terrain(float planetRadius) {
  enablePostprocess = false;
  terrain::planetRadius = planetRadius;

  Quadnode::gm = GenerationManager(160);
  ubo.nodesData.storage(nullptr, TERRAIN_MAX_NODES * sizeof(NodeData), GL_DYNAMIC_STORAGE_BIT);
}

void Terrain::update(const Camera* cam) {
  heightScale = planetRadius * planetRadiusPercent;
  leafs.clear();

  for (Quadnode& quadtree : quadtrees) {
    quadtree.insert(cam->getPosition());
    quadtree.gatherLeafs(leafs);
  }

  assert(leafs.size() <= TERRAIN_MAX_NODES);
  ubo.nodesData.updateSubData(leafs.data(), leafs.size() * sizeof(NodeData));
  chunkMesh.setInstanceCount(leafs.size());
}

void Terrain::reload() {
  Quadnode::gm.freeSlotAll();
  Quadnode::gm.update();

  quadtrees[0] = {Quadnode::Right};
  quadtrees[1] = {Quadnode::Left};
  quadtrees[2] = {Quadnode::Top};
  quadtrees[3] = {Quadnode::Bottom};
  quadtrees[4] = {Quadnode::Front};
  quadtrees[5] = {Quadnode::Back};
}

void Terrain::draw(const Camera* cam, Shader& shader) const {
  mat4 localView = cam->getLocalView(vec3(0.f));
  mat4 localTranslation = glm::translate(mat4(1.f), terrain::planetPos - cam->getPosition());

  shader.setUniform1f("u_planetRadius", planetRadius);
  shader.setUniform1f("u_heightScale", heightScale);
  shader.setUniform1f("u_seaThreshold", seaThreshold);
  shader.setUniform1f("u_sandThreshold", sandThreshold);
  shader.setUniform1f("u_mountainThreshold", mountainThreshold);
  shader.setUniformMatrix4f("u_localView", localView);
  shader.setUniformMatrix4f("u_localTranslation", localTranslation);

  Quadnode::gm.bindTexture();
  ubo.nodesData.bindBase(0);
  chunkMesh.draw(cam, shader);
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
  shader.setUniform1f("u_planetRadius", planetRadius);
  shader.setUniform1f("u_atmosphereScale", atmosphereScale);
  shader.setUniform1i("u_enable", enablePostprocess);
  shader.setUniformMatrix4f("u_invPV", glm::inverse(camProj * localView));

  Mesh::drawScreen(cam, shader);
}

} // terrain

