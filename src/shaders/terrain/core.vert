#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec4 v_lightSpacePos;
out vec3 v_viewDir;
out vec2 v_uv;
out flat uint v_meshIdx;

uniform mat4 u_model;
uniform mat4 u_camPV;
uniform mat4 u_lightSpace;
uniform vec3 u_camPos;
uniform vec2 u_gridAnchor;
uniform float u_heightScale;
uniform float u_chunkSize;
uniform int u_rings;

layout(binding = 0) uniform sampler2D u_texBufferA;

void main() {
  ivec2 offset;
  offset.x = gl_InstanceID % 4;
  offset.y = gl_InstanceID / 4;

  float scaleMax = exp2(u_rings);
  float uvScale = 0.25f / scaleMax;
  vec2 pos = (a_uv + offset - 2.f) * u_chunkSize;
  vec2 absoluteWorldXZ = pos + u_gridAnchor; // instead of (a_pos * 0.5f + 0.5f)

  vec2 uv = (absoluteWorldXZ + 2.f * scaleMax) * uvScale;

  float h = textureLod(u_texBufferA, uv, 0.f).r;
  vec4 worldPos = vec4(absoluteWorldXZ.x, h * u_heightScale, absoluteWorldXZ.y, 1.f);

  v_viewDir = u_camPos - worldPos.xyz;
  v_lightSpacePos = u_lightSpace * worldPos;
  v_uv = uv;
  v_meshIdx = 0;

  gl_Position = u_camPV * u_model * worldPos;
}

