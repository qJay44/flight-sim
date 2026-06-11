#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in int a_chunkIdx;

struct Chunk {
  vec2 worldPos;
  int index;
  int state;
  vec2 _pad;
};

uniform mat4 u_model;
uniform mat4 u_lightSpace;
uniform float u_heightScale;

layout(binding = 0) uniform sampler2DArray u_bufferA;

layout(std430, binding = 0) readonly buffer ChunkBuffer {
  Chunk chunks[];
};

void main() {
  Chunk chunk = chunks[a_chunkIdx];
  vec4 worldPos = u_model * vec4(a_pos, 1.f);
  float h = texture(u_bufferA, vec3(a_uv, chunk.index)).r;

  worldPos.xz += chunk.worldPos;
  worldPos.y = h * u_heightScale;

  gl_Position = u_lightSpace * worldPos;
}

