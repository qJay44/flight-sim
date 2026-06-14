#version 460 core

#include "common.glsl"

#define DRAINAGE_WIDTH 0.3

#define CLIFF_COLOR   vec3(0.22, 0.2,  0.2)
#define DIRT_COLOR    vec3(0.6,  0.5,  0.4)
#define GRASS_COLOR1  vec3(0.15, 0.3,  0.1)
#define GRASS_COLOR2  vec3(0.4,  0.5,  0.2)
#define SAND_COLOR    vec3(0.8,  0.7,  0.6)
#define SNOW_COLOR    vec3(1.f)
#define TREE_COLOR    vec3(0.12, 0.26, 0.1)
#define DEBRIS_COLOR  vec3(1.f)
#define AMBIENT_COLOR vec3(0.3,  0.5,  0.7)

out vec4 FragColor;

in vec4 v_lightSpacePos;
in vec3 v_worldPos;
in vec3 v_viewDir;
in vec2 v_uv;
in flat int v_chunkIdx;

struct Chunk {
  vec2 worldPos;
  int index;
  int state;
  vec2 _pad;
};

uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform vec3 u_camPos;
uniform vec3 u_debugChunkGroupColor;
uniform vec2 u_cliffEdges;
uniform vec2 u_dirtEdges;
uniform vec2 u_snowEdges;
uniform vec2 u_sandEdges;
uniform vec2 u_grass0Edges;
uniform vec2 u_grass1Edges;
uniform vec2 u_grass2Edges;
uniform float u_showChunkGroups;
uniform float u_heightScale;
uniform float u_appearance;
uniform float u_sunFocus;
uniform float u_sunIntensity;

layout(binding = 0) uniform sampler2DArray u_bufferA;
layout(binding = 1) uniform sampler2DArray u_bufferB;
layout(binding = 2) uniform sampler2DShadow u_shadowMap;

layout(std430, binding = 0) readonly buffer ChunkBuffer {
  Chunk chunks[];
};

void applyMask(inout vec3 src, vec3 dst, float b) {
  src = dst * b + src * (1.f - b);
}

vec3 getNormal(vec2 texelSize, int texSlot, float heightCenter) {
  float hr = texture(u_bufferA, vec3(v_uv + vec2(texelSize.x, 0.f), texSlot)).r;
  float hu = texture(u_bufferA, vec3(v_uv + vec2(0.f, texelSize.y), texSlot)).r;

  vec3 vr = vec3(texelSize.x, (hr - heightCenter) * u_heightScale, 0.f);
  vec3 vu = vec3(0.f, (hu - heightCenter) * u_heightScale, texelSize.y);

  return normalize(cross(vr, vu));
}

vec3 getSkyColor(vec3 normal) {
  float skyHemisphere = normal.y * 0.5f + 0.5f;

  vec3 skyColor = vec3(0.3f, 0.45f, 0.6f);  // Soft atmospheric blue
  vec3 groundColor = vec3(0.15f, 0.1f, 0.08f); // Muted dirt/rock brown

  return mix(groundColor, skyColor, skyHemisphere);
}

float getShadow(vec3 normal) {
  vec3 projCoords = v_lightSpacePos.xyz / v_lightSpacePos.w;
  projCoords = projCoords * 0.5f + 0.5f;

  float bias = max(0.005f * (1.f - dot(normal, u_lightDir)), 0.0005f);
  projCoords.z -= bias;

  return texture(u_shadowMap, projCoords);
}

void main() {
  const mat4 DITHER_MATRIX = mat4(
      0.0625, 0.5625, 0.1875, 0.6875,
      0.8125, 0.3125, 0.9375, 0.4375,
      0.2500, 0.7500, 0.1250, 0.6250,
      1.0000, 0.5000, 0.8750, 0.3750
  );

  ivec2 pixCoord = ivec2(gl_FragCoord.xy) % 4;
  float threshold = DITHER_MATRIX[pixCoord.x][pixCoord.y];

  if (u_appearance < threshold)
    discard;

  Chunk chunk = chunks[v_chunkIdx];
  vec2 texelSize = 1.f / textureSize(u_bufferA, 0).xy;
  float worldPosDist = length(v_viewDir);
  vec3 viewDir = v_viewDir / worldPosDist;

  vec4 bufA = texture(u_bufferA, vec3(v_uv, chunk.index));
  float height  = bufA.r;
  float ridge   = bufA.g;
  float trees   = bufA.b;
  float erosion = bufA.a * 2.f - 1.f;
  float drainage = saturate((1.f - saturate(ridge / DRAINAGE_WIDTH)) * 1.5f);
  float diff = u_camPos.y - height;
  vec3 normal = getNormal(texelSize, chunk.index, height);

  vec4 breakupTex = texture(u_bufferB, vec3(v_uv, chunk.index));
  float breakup = breakupTex.x;

  float occlusion = saturate(erosion + 0.5f);
  float tree = max(0.f, trees * 2.f - 1.f);

  float cliffMask  = smoothstep(u_cliffEdges.x, u_cliffEdges.y, height);
  float dirtMask   = smoothstep_inv(u_dirtEdges.x, u_dirtEdges.y, occlusion + breakup * 1.5f);
  float snowMask   = smoothstep(u_snowEdges.x, u_snowEdges.y, height + breakup * 0.1f);
  float sandMask   = smoothstep_inv(WATER_HEIGHT + u_sandEdges.x, WATER_HEIGHT + u_sandEdges.y, height + breakup * 0.01f);
  float grassMask0 = smoothstep(u_grass0Edges.x, u_grass0Edges.y, height - erosion * 0.05f + breakup * 0.3f);
  float grassMask1 = smoothstep_inv(GRASS_HEIGHT + u_grass1Edges.x, GRASS_HEIGHT + u_grass1Edges.y, height + 0.01f + (occlusion - 0.8f) * 0.05f - breakup * 0.02f);
  float grassMask2 = smoothstep(u_grass2Edges.x, u_grass2Edges.y, 1.f - (1.f - normal.y) * (1.f - trees) + breakup * 0.1f);
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
  vec3 color = diffuseColor * Fd_Lambert() * getSkyColor(normal);
  color *= occlusion;

  vec3 f0 = vec3(0.04f);
  float smoothness = 0.1f;
  float shadow = getShadow(normal);
  color += Shade(diffuseColor, f0, smoothness, normal, -viewDir, u_lightDir, u_lightColor * shadow);

  // Bounce
  color += diffuseColor * u_lightColor
    * (dot(normal, u_lightDir * vec3(1.f, -1.f, 1.f)) * 0.5f + 0.5f)
    * Fd_Lambert() / PI;

  applyMask(color, u_debugChunkGroupColor, u_showChunkGroups);

  FragColor = vec4(color, 1.f);
}

