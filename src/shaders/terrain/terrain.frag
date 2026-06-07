#version 460 core

#include "common.glsl"

out vec4 FragColor;

in vec2 v_uv;
in flat int v_instance;

struct Chunk {
  vec2 worldPos;
  int textureSlot;
  int padding;
};

uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform vec3 u_camPos;
uniform vec3 u_debugChunkGroupColor;
uniform float u_showChunkGroups;

layout(binding = 0) uniform sampler2DArray u_bufferA;
layout(binding = 1) uniform sampler2DArray u_bufferB;

layout(std430, binding = 2) readonly buffer ChunkBuffer {
  Chunk chunks[];
};

void applyMask(inout vec3 src, vec3 dst, float b) {
  src = dst * b + src * (1.f - b);
}

void main() {
  Chunk chunk = chunks[v_instance];

  vec4 bufA = texture(u_bufferA, vec3(v_uv, chunk.textureSlot));
  vec3 normal = bufA.yzz;
  normal.y = sqrt(1.f - dot(normal.xz, normal.xz));
  normal = normalize(normal.xyz);

  float diffuse = max(0.f, dot(normal, u_lightDir));
  vec3 finalColor = mix(GRASS_COLOR1, DIRT_COLOR, bufA.w);
  finalColor *= diffuse;

  applyMask(finalColor, u_debugChunkGroupColor, u_showChunkGroups);

  FragColor = vec4(finalColor, 1.f);
}

