#version 460 core

#include "../common.glsl"

layout(location = 0) in vec3 a_pos;

out vec3 v_worldPos;
out vec3 v_normal;
out vec2 v_uv;
out flat int v_layerIdx;

uniform mat4 u_proj;
uniform mat4 u_localView;
uniform mat4 u_localTranslation;
uniform float u_camFar;
uniform float u_planetRadius;
uniform float u_heightScale;

layout(binding = 0) uniform sampler2DArray u_texArray;

layout(std140, binding = 0) uniform NodesDataBlock {
  NodeData nodesData[TERRAIN_MAX_NODES];
};

void main() {
  NodeData node = nodesData[gl_InstanceID];
  ivec2 texSize = textureSize(u_texArray, 0).xy;
  vec2 texelSize = 1.f / texSize;

  vec2 nodePos = a_pos.xz * node.extents + node.center;
  vec2 uv = a_pos.xz * 0.5f + 0.5f;
  vec3 sphereDir = normalize(cubeToSphere(nodePos, node.faceIdx));

  vec4 terrainData = texture(u_texArray, vec3(uv, node.texLayerIdx));
  float height = terrainData.a;

  vec3 localSpherePos = sphereDir * (u_planetRadius + height);
  vec4 worldPos = vec4(localSpherePos, 1.f);

  v_worldPos = worldPos.xyz;
  v_normal = sphereDir;
  v_uv = uv;
  v_layerIdx = node.texLayerIdx;

	gl_Position = u_proj * (u_localView * (u_localTranslation * worldPos));

  float C = 0.001;
  float distNorm = log(C * gl_Position.w + 1.0) / log(C * u_camFar + 1.0);
  gl_Position.z = (distNorm * 2.0 - 1.0) * gl_Position.w;
}

