#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 v_uv;
out flat int v_instance;

struct Chunk {
  vec2 worldPos;
  int textureSlot;
  int padding;
};

uniform mat4 u_model;
uniform mat4 u_camPV;
uniform float u_heightScale;
uniform float u_chunkSize;
uniform int u_bufferSize;

layout(binding = 0) uniform sampler2DArray u_bufferA;

layout(std430, binding = 2) readonly buffer ChunkBuffer {
  Chunk chunks[];
};

void main() {
  Chunk chunk = chunks[gl_InstanceID];
  vec4 worldPos = u_model * vec4(a_pos, 1.f);

  worldPos.xz += chunk.worldPos;

  vec2 te = vec2(1.f / 1024.f);
  vec2 d = te * 0.5f + a_uv * (1.f - te);
  float h = texture(u_bufferA, vec3(d, chunk.textureSlot)).r;
  worldPos.y = exp2(h * u_heightScale);

  v_uv = a_uv;
  v_instance = gl_InstanceID;

	gl_Position = u_camPV * worldPos;
}

