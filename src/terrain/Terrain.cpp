#include "Terrain.hpp"

#include <cassert>

#include "glm/ext/matrix_transform.hpp"
#include "glm/matrix.hpp"
#include "global.hpp"
#include "shared.hpp"
#include "quadtree.hpp"

namespace terrain {

Terrain::Terrain(float planetRadius) {
  enablePostprocess = false;
  terrain::planetRadius = planetRadius;
  float planetRadiusInv = 1.f / planetRadius;

  waveScale = planetRadiusInv * 10.f;
  waterRadiusScale = 1.f + planetRadiusInv * 120.f;

  Quadnode::gm = GenerationManager(160);
  ubo.nodesData.storage(nullptr, TERRAIN_MAX_NODES * sizeof(NodeData), GL_DYNAMIC_STORAGE_BIT);
}

void Terrain::update(const Camera* cam) {
  heightScale = planetRadius * planetRadiusPercent;
  leafs.clear();
  Quadnode::gm.update();
  Quadnode::gm.generateWater();

  auto taskQt = global::profiler->startScopedTask("Quadtree pass");
  for (Quadnode& quadtree : quadtrees) {
    quadtree.insert(cam->getPosition());
    quadtree.gatherLeafs(leafs);
  }
  taskQt.end();

  assert(leafs.size() <= TERRAIN_MAX_NODES);
  ubo.nodesData.updateSubData(leafs.data(), leafs.size() * sizeof(NodeData));
  chunkMesh.setInstanceCount(leafs.size());
  waterMesh.setInstanceCount(leafs.size());
}

void Terrain::reload() {
  Quadnode::gm.freeSlotAll();

  quadtrees[0] = {Quadnode::Right};
  quadtrees[1] = {Quadnode::Left};
  quadtrees[2] = {Quadnode::Top};
  quadtrees[3] = {Quadnode::Bottom};
  quadtrees[4] = {Quadnode::Front};
  quadtrees[5] = {Quadnode::Back};
}

void Terrain::drawTerrain(const Camera* cam, Shader& shader) const {
  setCommonUniforms(cam, shader);

  Quadnode::gm.bindTextures();
  ubo.nodesData.bindBase(0);

  chunkMesh.draw(cam, shader);
}

void Terrain::drawWater(const Camera* cam, Shader& shader) const {
  setCommonUniforms(cam, shader);

  shader.setUniformMatrix4f("u_camProjInv", glm::inverse(cam->getProj()));
  shader.setUniform1f("u_waveScale", waveScale);
  shader.setUniform1f("u_waveHeight", waveHeight);
  shader.setUniform1f("u_radiusScale", waterRadiusScale);

  Quadnode::gm.bindTextures();
  ubo.nodesData.bindBase(0);

  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  waterMesh.draw(cam, shader);

  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
}

void Terrain::drawPostprocess(const Camera* cam, Shader& shader) const {
  const mat4& camProj = cam->getProj();
  const mat4& localView = cam->getLocalView(vec3(0.f));

  // TODO: Atmosphere
  shader.setUniform1f("u_planetRadius", planetRadius);
  shader.setUniform1i("u_enable", enablePostprocess);
  shader.setUniformMatrix4f("u_invPV", glm::inverse(camProj * localView));

  Mesh::drawScreen(cam, shader);
}

void Terrain::setCommonUniforms(const Camera* cam, Shader& shader) const {
  vec3 planetCameraOffset = terrain::planetPos - cam->getPosition();
  mat4 localView = cam->getLocalView(vec3(0.f));
  mat4 localTranslation = glm::translate(mat4(1.f), planetCameraOffset);

  shader.setUniform3f("u_planetCameraOffset", planetCameraOffset);
  shader.setUniform1f("u_planetRadius", planetRadius);
  shader.setUniform1f("u_heightScale", heightScale);
  shader.setUniform1f("u_seaThreshold", seaThreshold);
  shader.setUniform1f("u_sandThreshold", sandThreshold);
  shader.setUniform1f("u_mountainThreshold", mountainThreshold);
  shader.setUniformMatrix4f("u_localView", localView);
  shader.setUniformMatrix4f("u_localViewInv", glm::inverse(localView));
  shader.setUniformMatrix4f("u_localTranslation", localTranslation);
}

} // terrain

