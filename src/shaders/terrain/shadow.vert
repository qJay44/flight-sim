#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_lightSpace;
uniform float u_heightScale;

layout(binding = 0) uniform sampler2D u_texBufferA;

void main() {
  ivec2 offset;
  offset.x = gl_InstanceID % 4;
  offset.y = gl_InstanceID / 4;

  float scaleMax = exp2(u_rings);
  float uvScale = 0.25f / scaleMax;
  vec2 pos = a_uv + offset - 2.f;
  vec2 uv = (pos + 2.f * scaleMax) * uvScale;

  float h = texture(u_texBufferA, uv).r;
  vec4 worldPos = vec4(pos.x, h * u_heightScale, pos.y, 1.f);

  gl_Position = u_lightSpace * u_model * worldPos;
}

