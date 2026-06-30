#pragma once

#include <cassert>

#include "GenerationManager.hpp"
#include "NodeData.hpp"

namespace terrain {

struct Quadnode {
  static int maxDepth;
  static float splitThreshold;
  static GenerationManager gm;

  enum Face {
    Right = 0,
    Left = 1,
    Top = 2,
    Bottom = 3,
    Front = 4,
    Back = 5,
  };

  Face face;
  vec2 center{0.f};
  float extents{1.f}; // Distance from node center to its edges
  int depth = 1;
  int texLayerIdx = -1;
  Quadnode* children[4]{};

  Quadnode(Face face); // Root node
  ~Quadnode();

  bool isLeaf() const;
  void insert(vec3 camPos);
  void gatherLeafs(std::vector<NodeData>& leafs);

private:
  Quadnode(Face face, vec2 center, float extents, int depth);

  void split();
  void merge();

  vec3 cubeToSphere() const;

  float calculateSplitPriority(vec3 camPos) const;
};

} // namespace terrain

