#version 460 core

#include "common.glsl"

#define DRAINAGE_WIDTH 0.3

#define CLIFF_COLOR  vec3(0.22, 0.2, 0.2)
#define DIRT_COLOR   vec3(0.6, 0.5, 0.4)
#define GRASS_COLOR1 vec3(0.15, 0.3, 0.1)
#define GRASS_COLOR2 vec3(0.4, 0.5, 0.2)
#define SAND_COLOR   vec3(0.8, 0.7, 0.6)
#define SNOW_COLOR   vec3(1.f)
#define TREE_COLOR   vec3(0.12, 0.26, 0.1)
#define DEBRIS_COLOR vec3(1.f)
#define AMBIENT_COLOR vec3(0.3, 0.5, 0.7)

out vec4 FragColor;

in vec3 v_worldPos;
in vec3 v_viewDir;
in vec2 v_uv;
in flat int v_instance;

struct Chunk {
  vec2 worldPos;
  int textureSlot;
};

uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform vec3 u_camPos;
uniform vec3 u_debugChunkGroupColor;
uniform float u_showChunkGroups;
uniform float u_heightScale;

layout(binding = 0) uniform sampler2DArray u_bufferA;
layout(binding = 1) uniform sampler2DArray u_bufferB;
layout(binding = 2) uniform samplerCube u_texSkybox;

layout(std430, binding = 0) readonly buffer ChunkBuffer {
  Chunk chunks[];
};

void applyMask(inout vec3 src, vec3 dst, float b) {
  src = dst * b + src * (1.f - b);
}

vec3 getNormal(vec2 texelSize, int texSlot, float heightCenter) {
  float hr = texture(u_bufferA, vec3(v_uv + vec2(texelSize.x, 0.f), texSlot)).r;
  float hu = texture(u_bufferA, vec3(v_uv + vec2(0.f, texelSize.y), texSlot)).r;

  vec3 vr = vec3(texelSize.x, 0.f, hr - heightCenter);
  vec3 vu = vec3(0.f, texelSize.x, hu - heightCenter);

  vec3 n = normalize(cross(vr, vu));
  n.x = -n.x;

  return n;
}

vec3 getSkyColor(vec3 normal) {
  return texture(u_texSkybox, normal).rgb;
}

void main() {
  Chunk chunk = chunks[v_instance];
  vec2 texelSize = 1.f / textureSize(u_bufferA, 0).xy;
  float worldPosDist = length(v_viewDir);
  vec3 viewDir = v_viewDir / worldPosDist;

  vec4 bufA = texture(u_bufferA, vec3(v_uv, chunk.textureSlot));
  float height  = bufA.r;
  float ridge   = bufA.g;
  float trees   = bufA.g;
  float erosion = bufA.a * 2.f - 1.f;
  float drainage = saturate((1.f - saturate(ridge / DRAINAGE_WIDTH)) * 1.5f);
  float diff = u_camPos.y - height;
  vec3 normal = getNormal(texelSize, chunk.textureSlot, height);

  vec4 breakupTex = texture(u_bufferB, vec3(v_uv, chunk.textureSlot));
  float breakup = breakupTex.x;

  float occlusion = saturate(erosion + 0.5f);
  float tree = max(0.f, trees * 2.f - 1.f);

  float cliffMask  = smoothstep(0.4f, 0.52f, height);
  float dirtMask   = smoothstep_inv(0.6f, 0.f, occlusion + breakup * 1.5f);
  float snowMask   = smoothstep(0.53f, 0.6f, height + breakup * 0.1f);
  float sandMask   = smoothstep_inv(WATER_HEIGHT + 0.005f, WATER_HEIGHT, height + breakup * 0.01f);
  float grassMask0 = smoothstep(0.4f, 0.6f, height - erosion * 0.05f + breakup * 0.3f);
  float grassMask1 = smoothstep_inv(GRASS_HEIGHT + 0.05f, GRASS_HEIGHT + 0.02f, height + 0.01f + (occlusion - 0.8f) * 0.05f - breakup * 0.02f);
  float grassMask2 = smoothstep(0.8f, 1.f, 1.f - (1.f - normal.y) * (1.f - trees) + breakup * 0.1f);
  float treeMask   = saturate(trees * 2.2f - 0.8f) * 0.6f;

  vec3 grassMix = mix(GRASS_COLOR1, GRASS_COLOR2, grassMask0);

  vec3 diffuseColor = CLIFF_COLOR * cliffMask;
  diffuseColor = mix(diffuseColor, DIRT_COLOR, dirtMask);
  diffuseColor = mix(diffuseColor, SNOW_COLOR, snowMask);
  diffuseColor = mix(diffuseColor, SAND_COLOR, sandMask);
  diffuseColor = mix(diffuseColor, grassMix, grassMask1 * grassMask2);
  diffuseColor = mix(diffuseColor, TREE_COLOR * pow(trees, 8.f), treeMask);
  diffuseColor *= 1.f + breakup * 0.5f;

  // Drainage (rivers, creeks, debris flow)
  diffuseColor = mix(diffuseColor, DEBRIS_COLOR, drainage);

  // Ambient
  vec3 color = diffuseColor * getSkyColor(normal) * Fd_Lambert();
  color *= occlusion;

  // Direct
  vec3 f0 = vec3(0.04f);
  float smoothness = 0.f;
  color += Shade(diffuseColor, f0, smoothness, normal, -viewDir, u_lightDir, u_lightColor);

  // Bounce
  color += diffuseColor * u_lightColor
    * (dot(normal, u_lightDir * vec3(1.f, -1.f, 1.f)) * 0.5f + 0.5f)
    * Fd_Lambert() / PI;

  applyMask(color, u_debugChunkGroupColor, u_showChunkGroups);

  FragColor = vec4(color, 1.f);
}

