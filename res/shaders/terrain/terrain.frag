#version 460 core

#include "../common.glsl"
#include "terrain.glsl"

layout(location = 0) out vec4 FragColor0;
layout(location = 1) out vec4 FragColor1;

in vec3 v_worldPos;
in vec3 v_sphereDir;
in vec3 v_faceTangent;
in vec2 v_uv;
in flat int v_texLayer;

layout(binding = 0) uniform sampler2DArray u_texArrayComputeData;
layout(binding = 1) uniform sampler2DArray u_texArrayAlbedo;
layout(binding = 2) uniform sampler2DArray u_texArrayNormal;
layout(binding = 3) uniform sampler2DArray u_texArrayORMH;

uniform vec3 u_lightDir;
uniform float u_planetRadius;
uniform float u_seaThreshold;
uniform float u_sandThreshold;
uniform float u_mountainThreshold;
uniform float u_heightScale;

Material materialUnpack(vec2 uv, int slot, mat3 tbn) {
  vec4 ormh = texture(u_texArrayORMH, vec3(uv, slot));

  Material m;
  m.albedo    = texture(u_texArrayAlbedo, vec3(uv, slot)).rgb;
  m.normal    = normalize(tbn * texture(u_texArrayNormal, vec3(uv, slot)).rgb);
  m.ao        = ormh.r;
  m.roughness = ormh.g;
  m.metallic  = ormh.b;
  m.height    = ormh.a;

  return m;
}

Material materialMix(Material m1, Material m2, float t) {
  m1.albedo    = mix(m1.albedo,    m2.albedo,    t);
  m1.normal    = mix(m1.normal,    m2.normal,    t);
  m1.ao        = mix(m1.ao,        m2.ao,        t);
  m1.roughness = mix(m1.roughness, m2.roughness, t);
  m1.metallic  = mix(m1.metallic,  m2.metallic,  t);
  m1.height    = mix(m1.height,    m2.height,    t);

  return m1;
}

mat3 getTBN(vec3 normal) {
  vec3 N = normalize(v_sphereDir);
  vec3 T = normalize(normal - dot(normal, N) * N);
  vec3 B = normalize(cross(T, N));

  return mat3(T, B, N);
}

void main() {
  vec4 terrainData = texture(u_texArrayComputeData, vec3(v_uv, v_texLayer));
  float height = terrainData.a / u_heightScale;
  vec3 computeNormal = terrainData.rgb;
  mat3 tbn = getTBN(computeNormal);

  // Calculate slope (steepness)
  // We check how much the surface normal aligns with the outward radial direction of the planet.
  // dot = 1.0 means perfectly flat ground. dot = 0.0 means a vertical 90-degree cliff wall.
  vec3 radialDir = normalize(v_sphereDir);
  float flatness = dot0(computeNormal, radialDir);
  float slope = 1.f - flatness;

  float sandToGrassWeight = smoothstep(u_seaThreshold, u_sandThreshold, height);
  float slopeWeight = smoothstep(0.15f, 0.3f, slope);
  float snowWeight = smoothstep(u_mountainThreshold, 0.90f, height);
  float weights[4] = float[4](0.f, sandToGrassWeight, slopeWeight, snowWeight);

  Material surfaceMaterial = materialUnpack(v_uv, 0, tbn);

  for (int i = 1; i < 4; i++)
    if (weights[i] > 0.f)
      surfaceMaterial = materialMix(surfaceMaterial, materialUnpack(v_uv, i, tbn), weights[i]);

  float diffuse = dot0(surfaceMaterial.normal, u_lightDir);
  float ambient = 0.01f;

  vec3 color = surfaceMaterial.albedo * (diffuse + ambient);

  FragColor0 = vec4(color, 1.f);
  FragColor1 = vec4(color, 1.f);
}

