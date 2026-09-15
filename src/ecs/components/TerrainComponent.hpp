#pragma once

#include "../../core/math/terrain/NodeData.hpp"
#include "../../core/math/terrain/quadtree.hpp"
#include "../../gfx/BufferObject.hpp"

namespace ecs::component::terrain {

using namespace core::math::terrain;

struct TerrainComponent {
  vec3 planetPos = vec3(0.f);
  float planetRadius = 1.f;
  float heightScale = 1.f;

  float planetRadiusPercent = 0.02f;
  float seaThreshold = 0.05f;     // Percentage of [heightScale]
  float sandThreshold = 0.08f;    // Percentage of [heightScale]
  float mountainThreshold = 0.6f; // Percentage of [heightScale]

  float waveScale = 1.f;
  float waterRadiusScale = 1.f;
  float foamSharpness = 1.f;

  std::vector<NodeData> leafs;
  std::array<Quadnode, 6> quadtrees{
    Quadnode::Right,
    Quadnode::Left,
    Quadnode::Top,
    Quadnode::Bottom,
    Quadnode::Front,
    Quadnode::Back,
  };

  struct {
    gfx::BufferObject nodesData;
  } ubo;
};

} // namespace terrain

