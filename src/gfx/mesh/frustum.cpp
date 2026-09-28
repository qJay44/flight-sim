#include "frustum.hpp"

#include "vertex.hpp"
#include "MeshData.hpp"
#include "../../core/math/frustum/Frustum.hpp"

namespace gfx::frustum {

namespace {

std::vector<vertex::P> createVertices(const core::Camera& cam) {
  core::math::frustum::Frustum frustum(cam);

  vec3 camLeft = normalize(cross(-cam.orientation, cam.up));
  vec3 camRight = -camLeft;

  float aspectRatio = cam.cachedProj[1][1] / cam.cachedProj[0][0];
  float farVSideHalf = cam.farPlane * tanf(cam.fov) * 0.5f;
  float farHSideHalf = farVSideHalf * aspectRatio;
  vec3 farPos = cam.position + cam.orientation * cam.farPlane;
  vec3 farTR = farPos + camRight * farHSideHalf +  cam.up * farVSideHalf;
  vec3 farTL = farPos + camLeft  * farHSideHalf +  cam.up * farVSideHalf;
  vec3 farBL = farPos + camLeft  * farHSideHalf + -cam.up * farVSideHalf;
  vec3 farBR = farPos + camRight * farHSideHalf + -cam.up * farVSideHalf;

  float nearVSideHalf = cam.nearPlane * tanf(cam.fov) * 0.5f;
  float nearHSideHalf = nearVSideHalf * aspectRatio;
  vec3 nearPos = cam.position + cam.orientation * cam.nearPlane;
  vec3 nearTR = nearPos + camRight * nearHSideHalf +  cam.up * nearVSideHalf;
  vec3 nearTL = nearPos + camLeft  * nearHSideHalf +  cam.up * nearVSideHalf;
  vec3 nearBL = nearPos + camLeft  * nearHSideHalf + -cam.up * nearVSideHalf;
  vec3 nearBR = nearPos + camRight * nearHSideHalf + -cam.up * nearVSideHalf;

  vec3 centerTR = (farTR + nearTR) * 0.5f;
  vec3 centerTL = (farTL + nearTL) * 0.5f;
  vec3 centerBR = (farBR + nearBR) * 0.5f;
  vec3 centerBL = (farBL + nearBL) * 0.5f;

  vec3 centerTop    = (centerTR + centerTL) * 0.5f;
  vec3 centerBottom = (centerBR + centerBL) * 0.5f;
  vec3 centerRight  = (centerTR + centerBR) * 0.5f;
  vec3 centerLeft   = (centerTL + centerBL) * 0.5f;

  std::vector<vertex::P> vertices {
    // Near plane
    {nearTR},
    {nearTL},
    {nearBL},
    {nearBR},
    // Far plane
    {farTR},
    {farTL},
    {farBL},
    {farBR},
    // Normals
    {centerTop},
    {centerTop + frustum.topFace.normal},
    {centerBottom},
    {centerBottom + frustum.bottomFace.normal},
    {centerRight},
    {centerRight + frustum.rightFace.normal},
    {centerLeft},
    {centerLeft + frustum.leftFace.normal},
    {nearPos},
    {nearPos + frustum.nearFace.normal},
    {farPos},
    {farPos + frustum.farFace.normal},
  };

  return vertices;
}

} // namespace

Mesh create(const core::Camera& cam) {
  std::vector<vertex::P> vertices = createVertices(cam);

  std::vector<GLuint> indices {
    // Near plane
    0, 1,
    1, 2,
    2, 3,
    3, 0,
    // Far plane
    4, 5,
    5, 6,
    6, 7,
    7, 4,
    // Connect corners
    0, 4,
    1, 5,
    2, 6,
    3, 7,
    // Normals
    8, 9,
    10, 11,
    12, 13,
    14, 15,
    16, 17,
    18, 19
  };

  MeshData data(vertices, indices);
  data.mode = GL_LINES;
  data.usage = GL_DYNAMIC_DRAW;

  return Mesh(data);
}

void update(Mesh& mesh, const core::Camera& cam) {
  auto vertices = createVertices(cam);
  mesh.updateDataBuffer(MeshData(vertices));
}

} // namespace gfx::frustum

