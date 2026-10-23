#version 460 core

#include "../common.glsl"
#include "../common-fs.glsl"
#include "terrain.glsl"

layout(location = 0) out vec4 FragColor0;
layout(location = 1) out vec4 FragColor1;

in vec3 v_worldPos;
in vec3 v_sphereDir;
in vec2 v_uv;
in flat int v_texLayer;

layout(binding = 0 ) uniform sampler2DArray u_texComputeData;
layout(binding = 1 ) uniform sampler2D u_texSandAlbedo;
layout(binding = 2 ) uniform sampler2D u_texSandNormal;
layout(binding = 3 ) uniform sampler2D u_texSandORMH;
layout(binding = 4 ) uniform sampler2D u_texGrassAlbedo;
layout(binding = 5 ) uniform sampler2D u_texGrassNormal;
layout(binding = 6 ) uniform sampler2D u_texGrassORMH;
layout(binding = 7 ) uniform sampler2D u_texRocksAlbedo;
layout(binding = 8 ) uniform sampler2D u_texRocksNormal;
layout(binding = 9 ) uniform sampler2D u_texRocksORMH;
layout(binding = 10) uniform sampler2D u_texSnowAlbedo;
layout(binding = 11) uniform sampler2D u_texSnowNormal;
layout(binding = 12) uniform sampler2D u_texSnowORMH;

uniform vec3 u_lightDir;
uniform float u_planetRadius;
uniform float u_seaThreshold;
uniform float u_sandThreshold;
uniform float u_mountainThreshold;
uniform float u_heightScale;

Material getMaterial(sampler2D texAlbedo, sampler2D texNormal, sampler2D texORMH, vec2 uv, mat3 tbn) {
  vec4 ormh = texture(texORMH, uv);

  Material m;
  m.albedo = texture(texAlbedo, uv).rgb;
  m.normal = normalize(tbn * texture(texNormal, uv).rgb);
  m.ao = ormh.r;
  m.roughness = ormh.g;
  m.metallic = ormh.b;
  m.height = ormh.a;

  return m;
}

Material materialMix(Material m1, Material m2, float t) {
  m1.albedo = mix(m1.albedo, m2.albedo, t);
  m1.normal = mix(m1.normal, m2.normal, t);
  m1.ao = mix(m1.ao, m2.ao, t);
  m1.roughness = mix(m1.roughness, m2.roughness, t);
  m1.metallic = mix(m1.metallic, m2.metallic, t);
  m1.height = mix(m1.height, m2.height, t);

  return m1;
}

void main() {
  vec4 terrainData = texture(u_texComputeData, vec3(v_uv, v_texLayer));
  float height = terrainData.a / u_heightScale;
  vec3 computeNormal = terrainData.rgb;

  mat3 tbn = getTBN(computeNormal, v_worldPos, v_uv);
  Material sand  = getMaterial(u_texSandAlbedo,  u_texSandNormal,  u_texSandORMH,  v_uv, tbn);
  Material grass = getMaterial(u_texGrassAlbedo, u_texGrassNormal, u_texGrassORMH, v_uv, tbn);
  Material rocks = getMaterial(u_texRocksAlbedo, u_texRocksNormal, u_texRocksORMH, v_uv, tbn);
  Material snow  = getMaterial(u_texSnowAlbedo,  u_texSnowNormal,  u_texSnowORMH,  v_uv, tbn);

  // Calculate slope (steepness)
  // We check how much the surface normal aligns with the outward radial direction of the planet.
  // dot = 1.0 means perfectly flat ground. dot = 0.0 means a vertical 90-degree cliff wall.
  vec3 radialDir = normalize(v_sphereDir);
  float flatness = dot0(computeNormal, radialDir);
  float slope = 1.f - flatness;

  Material surfaceMaterial;

  float sandToGrass = smoothstep(u_seaThreshold, u_sandThreshold, height);
  surfaceMaterial = materialMix(sand, grass, sandToGrass);

  float slopeWeight = smoothstep(0.15f, 0.3f, slope);
  surfaceMaterial = materialMix(surfaceMaterial, rocks, slopeWeight);

  float snowBlend = smoothstep(u_mountainThreshold, 0.70f, height);
  float cliffWeight = smoothstep(0.05f, 0.95f, slope);
  Material highAltitudeMaterial = materialMix(rocks, snow, cliffWeight);
  surfaceMaterial = materialMix(surfaceMaterial, highAltitudeMaterial, snowBlend);

  float diffuse = dot0(surfaceMaterial.normal, u_lightDir);
  float ambient = 0.01f;

  vec3 color = surfaceMaterial.albedo * (diffuse + ambient);

  FragColor0 = vec4(color, 1.f);
  FragColor1 = vec4(color, 1.f);
}

