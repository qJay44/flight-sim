#version 460 core

#include "common.glsl"

layout(location = 0) in vec3 a_pos;

out vec3 v_sphereDir;
out vec3 v_worldPos;
out vec2 v_uv;

uniform mat4 u_camProj;
uniform mat4 u_localView;
uniform mat4 u_localTranslation;
uniform float u_camFar;
uniform float u_planetRadius;
uniform float u_waveHeight;
uniform float u_waveScale;
uniform float u_radiusScale;

layout(binding = 1) uniform sampler2D u_texWaveMap;

layout(std140, binding = 0) uniform NodesDataBlock {
  NodeData nodesData[MAX_NODES];
};

float getTriplanarHeight(vec3 pos, vec3 normal, float scale) {
  vec2 uvX = pos.zy * scale;
  vec2 uvY = pos.xz * scale;
  vec2 uvZ = pos.xy * scale;

  float hx = texture(u_texWaveMap, uvX).a;
  float hy = texture(u_texWaveMap, uvY).a;
  float hz = texture(u_texWaveMap, uvZ).a;

  vec3 blend = abs(normal);
  blend = pow(blend, vec3(4.f));
  blend /= dot(blend, vec3(1.f));

  return hx * blend.x + hy * blend.y + hz * blend.z;
}

void main() {
  NodeData node = nodesData[gl_InstanceID];

  vec2 nodePos = a_pos.xz * node.extents + node.center;
  vec2 uv = a_pos.xz * 0.5f + 0.5f;
  vec3 sphereDir = normalize(cubeToSphere(nodePos, node.faceIdx));
  vec3 pos = sphereDir * u_planetRadius * u_radiusScale;
  float wave = getTriplanarHeight(pos, sphereDir, u_waveScale);

  vec4 worldPos = vec4(pos + sphereDir * wave * u_waveHeight, 1.f);

  v_sphereDir = sphereDir;
  v_worldPos = worldPos.xyz;
  v_uv = uv;

	gl_Position = u_camProj * (u_localView * (u_localTranslation * worldPos));

  float C = 0.001;
  float distNorm = log(C * gl_Position.w + 1.0) / log(C * u_camFar + 1.0);
  gl_Position.z = (distNorm * 2.0 - 1.0) * gl_Position.w;
}

