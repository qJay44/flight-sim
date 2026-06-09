#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec3 v_viewDir;
out vec3 v_worldPos;
out vec2 v_uv;
out flat int v_instance;

struct Chunk {
  vec2 worldPos;
  int textureSlot;
};

uniform mat4 u_model;
uniform mat4 u_camPV;
uniform vec3 u_camPos;
uniform float u_heightScale;
uniform float u_chunkSize;
uniform int u_bufferSize;

layout(binding = 0) uniform sampler2DArray u_bufferA;

layout(std430, binding = 0) readonly buffer ChunkBuffer {
  Chunk chunks[];
};

void main() {
  Chunk chunk = chunks[gl_InstanceID];
  vec4 worldPos = u_model * vec4(a_pos, 1.f);
  float h = texture(u_bufferA, vec3(a_uv, chunk.textureSlot)).r;

  worldPos.xz += chunk.worldPos;
  worldPos.y = h * u_heightScale;

  v_viewDir = u_camPos - worldPos.xyz;
  v_worldPos = worldPos.xyz;
  v_uv = a_uv;
  v_instance = gl_InstanceID;

	gl_Position = u_camPV * worldPos;
}

