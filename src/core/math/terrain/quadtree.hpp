#pragma once

#include "NodeData.hpp"

namespace core::math::terrain {

struct Quadnode {
  static int maxDepth;
  static float splitThreshold;

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

  void insert(vec3 camPos, float planetRadius, std::stack<int>& freedSlots);
  void gatherLeafs(std::vector<NodeData>& leafs);

private:
  Quadnode(Face face, vec2 center, float extents, int depth);

  bool isLeaf() const;
  void split();
  void merge(std::stack<int>& freedSlots);

  vec3 cubeToSphere() const;

  float calculateSplitPriority(vec3 camPos, float planetRadius) const;
};

} // namespace terrain

