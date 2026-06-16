#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec4 v_lightSpacePos;
out vec3 v_viewDir;
out vec2 v_uv;
out flat uint v_meshIdx;

uniform mat4 u_model;
uniform mat4 u_projView;
uniform mat4 u_lightSpace;
uniform vec3 u_camPos;
uniform vec2 u_gridAnchor;
uniform float u_heightScale;
uniform float u_chunkSize;
uniform int u_rings;

const int ringX[12] = int[](-2, -1,  0,  1, -2,  1, -2,  1, -2, -1,  0,  1);
const int ringY[12] = int[](-2, -2, -2, -2, -1, -1,  0,  0,  1,  1,  1,  1);

layout(binding = 0) uniform sampler2D u_texBufferA;

void main() {
  uint ring = gl_DrawID + 1;

  float scale = exp2(ring) * u_chunkSize;
  float scaleMax = exp2(u_rings);
  vec2 offset = vec2(ringX[gl_InstanceID], ringY[gl_InstanceID]);
  vec2 pos = (a_uv + offset) * scale; // instead of (a_pos * 0.5f + 0.5f)
  vec2 absoluteWorldXZ = pos + u_gridAnchor;

  // ring n    : 0 -> 0.25
  // ring n - 1: 0 -> 0.125

  float uvScale = 0.25f / scaleMax;
  vec2 uv = (absoluteWorldXZ + 2.f * scaleMax) * uvScale;

  float h = texture(u_texBufferA, uv).r;
  vec4 worldPos = vec4(absoluteWorldXZ.x, h * u_heightScale, absoluteWorldXZ.y, 1.f);

  v_viewDir = u_camPos - worldPos.xyz;
  v_lightSpacePos = u_lightSpace * worldPos;
  v_uv = uv;
  v_meshIdx = ring;

  gl_Position = u_projView * u_model * worldPos;
}

