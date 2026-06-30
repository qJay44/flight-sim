#version 460 core

#define MAX_NODES 512

layout(location = 0) in vec3 a_pos;

out vec3 v_sphereDir;
out vec2 v_uv;
out flat int v_layerIdx;

struct NodeData {
  vec2 center;
  float extents;
  int faceIdx;
  int texLayerIdx;
};

uniform mat4 u_camProj;
uniform mat4 u_localView;
uniform mat4 u_localTranslation;
uniform float u_camFar;
uniform float u_planetRadius;
uniform float u_heightScale;

layout(binding = 0) uniform sampler2DArray u_texArray;

layout(std140, binding = 0) uniform NodesDataBlock {
  NodeData nodesData[MAX_NODES];
};

vec3 cubeToSphere(vec2 pos, int faceIdx) {
  float u = pos.x;
  float v = pos.y;
  vec3 p;

  switch (faceIdx) {
    case 0: p = vec3( 1,  v,  u); break; // Right
    case 1: p = vec3(-1,  v, -u); break; // Left
    case 2: p = vec3( u,  1,  v); break; // Top
    case 3: p = vec3(-u, -1,  v); break; // Bottom
    case 4: p = vec3(-u,  v,  1); break; // Front
    case 5: p = vec3( u,  v, -1); break; // Back
  }

  return p;
}

void main() {
  NodeData node = nodesData[gl_InstanceID];

  vec2 nodePos = a_pos.xz * node.extents + node.center;
  vec2 uv = a_pos.xz * 0.5f + 0.5f;
  vec3 sphereDir = normalize(cubeToSphere(nodePos, node.faceIdx));

  vec4 terrainData = texture(u_texArray, vec3(uv, node.texLayerIdx));
  float height = terrainData.r;
  vec3 normal = terrainData.gba;

  vec3 localSpherePos = sphereDir * (u_planetRadius + height);
  vec4 worldPos = vec4(localSpherePos, 1.f);

  v_sphereDir = sphereDir;
  v_uv = uv;
  v_layerIdx = node.texLayerIdx;

	gl_Position = u_camProj * (u_localView * (u_localTranslation * worldPos));
}

