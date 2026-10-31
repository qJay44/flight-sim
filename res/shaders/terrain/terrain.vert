#version 460 core

#include "../common.glsl"
#include "terrain.glsl"

#define MESH_RESOLUTION 30
#define BASE_VERTEX_COUNT (MESH_RESOLUTION*MESH_RESOLUTION)

layout(location = 0) in vec3 a_pos;

out vec3 v_worldPos;
out vec3 v_sphereDir;
out vec3 v_faceTangent;
out vec2 v_uv;
out flat int v_texLayer;

uniform mat4 u_proj;
uniform mat4 u_localView;
uniform mat4 u_localTranslation;
uniform vec3 u_camPos;
uniform float u_camFar;
uniform float u_planetRadius;

layout(binding = 0) uniform sampler2DArray u_texArrayComputeData;

layout(std140, binding = 0) uniform NodesDataBlock {
  NodeData nodesData[TERRAIN_MAX_NODES];
};

void main() {
  NodeData node = nodesData[gl_InstanceID];

  vec2 nodePos = a_pos.xz * node.extents + node.center;
  vec2 uv = a_pos.xz * 0.5f + 0.5f;
  vec3 localNormal;
  vec3 sphereDir = cubeToSphere(nodePos, node.faceIdx, localNormal);
  vec3 radialDir = normalize(sphereDir);

  vec4 terrainData = texture(u_texArrayComputeData, vec3(uv, node.texLayerIdx));
  float height = terrainData.a;

  float sign = gl_VertexID >= BASE_VERTEX_COUNT ? -1.f : 1.f;
  vec3 localSpherePos = radialDir * (u_planetRadius + height * sign);
  vec4 worldPos = vec4(localSpherePos, 1.f);

  v_worldPos = worldPos.xyz;
  v_sphereDir = sphereDir;
  v_faceTangent = localNormal;
  v_uv = uv;
  v_texLayer = node.texLayerIdx;

	gl_Position = u_proj * (u_localView * (u_localTranslation * worldPos));
  gl_Position.z = linearDepthToLogDepth(gl_Position.w, u_camFar);
}

