#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_lightSpace;
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

  float eps = 1e-4f;
  bool isEdge = (a_uv.x < eps || a_uv.x > (1.f - eps) ||
                 a_uv.y < eps || a_uv.y > (1.f - eps));

  float skirtModifier = isEdge ? -5.f : 0.f;
  float h = textureLod(u_texBufferA, uv, 0.f).r;
  vec4 worldPos = vec4(absoluteWorldXZ.x, h * u_heightScale + skirtModifier, absoluteWorldXZ.y, 1.f);

  gl_Position = u_lightSpace * u_model * worldPos;
}

