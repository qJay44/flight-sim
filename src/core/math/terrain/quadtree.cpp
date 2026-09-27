#include "quadtree.hpp"

#include "../frustum/volumes/Sphere.hpp"

namespace core::math::terrain {

Quadnode::GlobalData Quadnode::g;
std::stack<int> Quadnode::freedTexLayerIdxs;

Quadnode::Quadnode(Face face) : face(face) {}

Quadnode::~Quadnode() {
  for (auto* child : children)
    if (child)
      delete child;
}

void Quadnode::newFrame(const GlobalData& globalData) {
  Quadnode::g = globalData;
  assert(freedTexLayerIdxs.empty());
}

void Quadnode::insert() {
  float score = calculateSplitPriority();

  if (score > g.splitThreshold && depth < g.maxDepth) {
    if (isLeaf())
      split();

    for (auto* child : children)
      child->insert();

  } else {
    if (!isLeaf())
      merge();
  }
}

void Quadnode::gatherLeafs(std::stack<Quadnode*>& leafs) {
  if (isLeaf())
    leafs.push(this);
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

void Quadnode::merge() {
  assert(children[0]);

  for (auto*& child : children) {
    if (child->texLayerIdx != -1)
      freedTexLayerIdxs.push(child->texLayerIdx);

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

float Quadnode::calculateSplitPriority() const {
  vec3 sphereDir = glm::normalize(cubeToSphere());
  vec3 elevatedCenter = sphereDir * g.planetRadius;

  float distance = glm::distance(g.camPos, elevatedCenter);
  float seaLevelRadius = extents * g.planetRadius * 1.4141f; // sqrt(2), diagonal length of the square

  auto frustumSphere = frustum::volume::Sphere(elevatedCenter, seaLevelRadius * 2.f);

  if (frustumSphere.isOnFrustum(*g.frustum))
    return seaLevelRadius / (distance + 0.001f);

  return 0.f;
}

} // namespace terrain

