#pragma once

#include "../engine/Camera.hpp"
#include "../engine/mesh/MeshElementsInstancing.hpp"
#include "GenerationManager.hpp"

struct gui;

namespace terrain {

class Terrain {
public:
  Terrain(int bufferSize, int rings);

  void update(const Camera* cam);

  void drawCore(const Camera* cam, Shader& shader) const;
  void drawCoreShadow(const Camera* cam, Shader& shader, const mat4& lightSpace) const;
  void drawRings(const Camera* cam, Shader& shader) const;
  void drawRingsShadow(const Camera* cam, Shader& shader, const mat4& lightSpace) const;
  void drawPostprocess(const Camera* cam, Shader& shader) const;

private:
  friend struct ::gui;

  struct {
    BufferObject cmd{GL_DRAW_INDIRECT_BUFFER};
  } ibo;

  struct DrawElementsIndirectCommand {
    uint count = 0;
    uint instanceCount = 0;
    uint firstIndex = 0;
    int baseVertex = 0;
    uint baseInstance = 0;
  };

  struct {
    vec2 gridAnchor{};
    vec2 globalOffsetUV{};
  } pending;

  int bufferSize;
  int rings;
  vec2 gridAnchor{};
  vec2 globalOffsetUV{};
  float chunkSize = 64.f;

  GenerationManager texManager;

  MeshElementsInstancing meshCore;
  MeshElements meshRing;

  float heightScale = 0.5f;
  float waterShoreScale = 0.06f;
  float waterRefractionScale = 0.082;
  float waterRefractionDistortScale = 0.05f;
  float waterNormalScaleUV = 0.25f;
  float waterNoiseScale = 0.15f;
  float foamEdge0 = 2.f;
  float foamEdge1 = -10.f;
  float fogDensity = 10.f;
  float fogDensityFalloff = 2e-4f;
  float horizonThickness = 50.f;
  float horizonFalloff = 10.f;

  vec2 cliffEdges  {0.40f , 0.52f};
  vec2 dirtEdges   {0.60f , 0.00f}; // inversed
  vec2 snowEdges   {0.72f , 0.81f};
  vec2 sandEdges   {0.005f, 0.00f}; // inversed, WATER_HEIGHT offset
  vec2 grass0Edges {0.75f , 1.00f};
  vec2 grass1Edges {0.30f , 0.00f}; // inversed, GRASS_HEIGHT offset
  vec2 grass2Edges {0.30f , 0.00f};

  bool debugLOD = false;

private:
  void setTerrainUniforms(Shader& shader) const;
};

} // terrain

