#pragma once

#include "../frustum/Frustum.hpp"

namespace core::math::terrain {

struct Quadnode {
  enum Face {
    Right = 0,
    Left = 1,
    Top = 2,
    Bottom = 3,
    Front = 4,
    Back = 5,
  };

  struct GlobalData {
    int maxDepth;
    float splitThreshold;
    float planetRadius;
    vec3 camPos;
    frustum::Frustum* frustum;
  };

  static std::stack<int> freedTexLayerIdxs;

  Face face;
  vec2 center{0.f};
  float extents{1.f}; // Distance from node center to its edges
  int depth = 1;
  int texLayerIdx = -1;
  Quadnode* children[4]{};

  Quadnode(Face face); // Root node
  ~Quadnode();

  void newFrame(const GlobalData& data);
  void insert();
  void gatherLeafs(std::stack<Quadnode*>& leafs);

private:
  static GlobalData g;

private:
  Quadnode(Face face, vec2 center, float extents, int depth);

  bool isLeaf() const;
  void split();
  void merge();

  vec3 cubeToSphere() const;

  float calculateSplitPriority() const;
};

} // namespace terrain

