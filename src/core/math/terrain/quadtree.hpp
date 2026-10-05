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
    float heightScale;
    vec3 camPos;
    frustum::Frustum* frustum;
  };

  struct RemoveData {
    u64 key;
    int texLayerIdx;
  };

  static std::stack<RemoveData> removedNodeDatas;

  Face face;
  vec2 center{0.f};
  float extents{1.f}; // Distance from node center to its edges
  bool onFrustum = true;
  int depth = 1;
  int texLayerIdx = -1;
  u64 key = 0;
  Quadnode* children[4]{};

  Quadnode(Face face); // Root node
  ~Quadnode();

  void newFrame(const GlobalData& data);
  void insert();
  void gatherLeafs(std::stack<Quadnode*>& leafs);

  static GlobalData g;
private:

private:
  Quadnode(Face face, vec2 center, float extents, int depth);

  void generateKey();

  bool isLeaf() const;
  void split();
  void merge();

  vec3 cubeToSphere() const;

  float calculateSplitPriority();
};

} // namespace terrain

