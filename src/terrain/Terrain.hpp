#pragma once

#include "../engine/Camera.hpp"
#include "../engine/mesh/meshes.hpp"
#include "../engine/mesh/MeshElementsInstancing.hpp"
#include "GenerationManager.hpp"
#include "ProfilerManager.hpp"
#include "quadtree.hpp"
#include "water/Tessendorf.hpp"

#define TERRAIN_MAX_NODES 512

struct gui;

namespace terrain {

class Terrain {
public:
  Terrain(float planetRadius);

  void update(const Camera* cam);
  void reload();

  void drawTerrain(const Camera* cam, Shader& shader) const;
  void drawWater(const Camera* cam, Shader& shader) const;
  void drawPostprocess(const Camera* cam, Shader& shader) const; // TODO: Move out postprocess into separate file

private:
  friend struct ::gui;

  struct {
    BufferObject nodesData{GL_UNIFORM_BUFFER};
  } ubo;

  Quadnode quadtrees[6] = {
    {Quadnode::Right},
    {Quadnode::Left},
    {Quadnode::Top},
    {Quadnode::Bottom},
    {Quadnode::Front},
    {Quadnode::Back},
  };

  std::vector<NodeData> leafs;
  MeshElementsInstancing chunkMesh = meshes::plane(128);
  MeshElementsInstancing waterMesh = meshes::plane(16);

  water::Tessendorf water;

  float planetRadiusPercent = 0.02f;
  float seaThreshold = 0.05f;     // Percentage of [heightScale]
  float sandThreshold = 0.08f;    // Percentage of [heightScale]
  float mountainThreshold = 0.6f; // Percentage of [heightScale]

  float waveScale = 1.f;
  float waterRadiusScale = 1.f;
  float foamSharpness = 1.f;

  ProfilerManager::Query queryDrawTerrain{"Terrain draw"};
  ProfilerManager::Query queryDrawWater{"Water draw"};

  bool enablePostprocess = true;

private:
  void setCommonUniforms(const Camera* cam, Shader& shader) const;
};

} // terrain

