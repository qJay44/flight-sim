#pragma once

namespace terrain {

struct NodeData {
  vec2 center;
  float extents;
  int faceIdx;
  int texLayerIdx = -1;
  vec3 _pad;
};
static_assert(sizeof(NodeData) % 16 == 0);

} // terrain

