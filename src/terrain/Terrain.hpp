#pragma once

#include "../engine/Camera.hpp"
#include "../engine/mesh/meshes.hpp"
#include "../engine/mesh/MeshElementsInstancing.hpp"
#include "GenerationManager.hpp"
#include "quadtree.hpp"

#define TERRAIN_MAX_NODES 512

struct gui;

namespace terrain {

class Terrain {
public:
  Terrain(float planetRadius);

  void update(const Camera* cam);
  void reload();

  void draw(const Camera* cam, Shader& shader) const;
  void drawPostprocess(const Camera* cam, Shader& shader) const;

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

  float planetRadiusPercent = 0.02f;
  float seaThreshold = 0.05f;     // Percentage of [heightScale]
  float sandThreshold = 0.08f;    // Percentage of [heightScale]
  float mountainThreshold = 0.6f; // Percentage of [heightScale]

  float waterShoreScale = 0.06f;
  float waterRefractionScale = 0.082;
  float waterRefractionDistortScale = 0.05f;
  float waterNormalScaleUV = 0.112f;
  float waterNoiseScale = 0.15f;
  float foamEdge0 = 2.f;
  float foamEdge1 = -10.f;
  float fogDensity = 10.f;
  float fogDensityFalloff = 2e-4f;
  float horizonThickness = 50.f;
  float horizonFalloff = 10.f;
  float atmosphereScale = 1.2f;

  vec2 cliffEdges  {0.40f , 0.52f};
  vec2 dirtEdges   {0.60f , 0.00f}; // inversed
  vec2 snowEdges   {0.72f , 0.81f};
  vec2 sandEdges   {0.005f, 0.00f}; // inversed, WATER_HEIGHT offset
  vec2 grass0Edges {0.75f , 1.00f};
  vec2 grass1Edges {0.30f , 0.00f}; // inversed, GRASS_HEIGHT offset
  vec2 grass2Edges {0.30f , 0.00f};

  bool enablePostprocess = true;
};

} // terrain

