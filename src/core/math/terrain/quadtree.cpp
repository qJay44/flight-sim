#include "quadtree.hpp"

namespace core::math::terrain {

int Quadnode::maxDepth = 8;
float Quadnode::splitThreshold = 0.5f;

Quadnode::Quadnode(Face face) : face(face) {}

Quadnode::~Quadnode() {
  for (auto* child : children)
    if (child)
      delete child;
}

void Quadnode::insert(vec3 camPos, float planetRadius, std::stack<int>& freedSlots) {
  float score = calculateSplitPriority(camPos, planetRadius);

  if (score > splitThreshold && depth < maxDepth) {
    if (isLeaf())
      split();

    for (auto* child : children)
      child->insert(camPos, planetRadius, freedSlots);
  } else {
    if (!isLeaf())
      merge(freedSlots);
  }
}

void Quadnode::gatherLeafs(std::vector<NodeData>& leafs) {
  if (isLeaf())
    leafs.push_back(NodeData{center, extents, face, texLayerIdx});
  else
    for (auto* child : children)
      child->gatherLeafs(leafs);
}

Quadnode::Quadnode(Face face, vec2 center, float extents, int depth)
  : face(face), center(center), extents(extents), depth(depth) {}

bool Quadnode::isLeaf() const { return children[0] == nullptr; }

void Quadnode::split() {
  assert(!children[0]);

  float ext = extents * 0.5f;
  vec2 tl{center.x - ext, center.y + ext};
  vec2 tr{center.x + ext, center.y + ext};
  vec2 bl{center.x - ext, center.y - ext};
  vec2 br{center.x + ext, center.y - ext};

  children[0] = new Quadnode(face, tl, ext, depth + 1);
  children[1] = new Quadnode(face, tr, ext, depth + 1);
  children[2] = new Quadnode(face, bl, ext, depth + 1);
  children[3] = new Quadnode(face, br, ext, depth + 1);
}

void Quadnode::merge(std::stack<int>& freedSlots) {
  assert(children[0]);

  for (auto*& child : children) {
    if (child->texLayerIdx != -1)
      freedSlots.push(child->texLayerIdx);

    delete child;
    child = nullptr;
  }
}

vec3 Quadnode::cubeToSphere() const {
  const float& u = center.x;
  const float& v = center.y;
  vec3 p;

  switch (face) {
    case Right:  p = { 1,  v,  u}; break;
    case Left:   p = {-1,  v, -u}; break;
    case Top:    p = { u,  1,  v}; break;
    case Bottom: p = { u, -1, -v}; break;
    case Front:  p = {-u,  v,  1}; break;
    case Back:   p = { u,  v, -1}; break;
    default: __builtin_unreachable();
  }

  return p;
}

float Quadnode::calculateSplitPriority(vec3 camPos, float planetRadius) const {
  vec3 sphereDir = glm::normalize(cubeToSphere());
  vec3 elevatedCenter = sphereDir * planetRadius;

  float distance = glm::distance(camPos, elevatedCenter);
  float seaLevelRadius = extents * planetRadius * 1.4141f; // sqrt(2), diagonal length of the square

  return seaLevelRadius / (distance + 0.001f);
}

} // namespace terrain

