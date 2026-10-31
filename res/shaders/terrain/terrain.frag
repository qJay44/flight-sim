#version 460 core

#include "../pbr.glsl"
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

uniform vec3 u_camPos;
uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform float u_lightIntensity;
uniform float u_planetRadius;
uniform float u_sandThreshold;
uniform float u_grassThreshold;
uniform float u_mountainThreshold;
uniform float u_heightScale;
uniform float u_heightScaleScale;

const vec3 cl[] = vec3[](
vec3(0.5789, 0.4854, 0.2684),
vec3(0.0358, 0.1726, 0.0230),
vec3(0.1332, 0.1190, 0.1056),
vec3(0.8933, 0.8933, 0.8933),
vec3(0.4420, 0.6814, 1.0000)
    );

Material materialUnpack(vec2 uv, int slot, mat3 tbn) {
  vec3 P = vec3(uv * 2.f, slot);
  // vec4 ormh = texture(u_texArrayORMH, P);
  // vec3 normal = texture(u_texArrayNormal, P).rgb * 2.f - 1.f;

  Material m  = materialInit;
  m.albedo    = cl[slot];
  // m.albedo    = texture(u_texArrayAlbedo, P).rgb;
  // m.normal    = normalize(tbn * normal);
  // m.ao        = ormh.r;
  // m.roughness = ormh.g;
  // m.metallic  = ormh.b;
  // m.height    = ormh.a;

  return m;
}

void materialMix(inout Material m1, in Material m2, float t) {
  m1.albedo    = mix(m1.albedo    , m2.albedo   , t);
  m1.normal    = mix(m1.normal    , m2.normal   , t);
  m1.ao        = mix(m1.ao        , m2.ao       , t);
  m1.roughness = mix(m1.roughness , m2.roughness, t);
  m1.metallic  = mix(m1.metallic  , m2.metallic , t);
  m1.height    = mix(m1.height    , m2.height   , t);
}

mat3 getTBN(vec3 normal) {
  vec3 N = normalize(normal);
  vec3 T = normalize(v_faceTangent - dot(v_faceTangent, N) * N);
  vec3 B = normalize(cross(T, N));

  return mat3(T, B, N);
}

void main() {
  vec3 radialDir = normalize(v_sphereDir);
  vec2 uv = v_uv;

  vec4 terrainData = texture(u_texArrayComputeData, vec3(uv, v_texLayer));
  float height = terrainData.a / u_heightScale;
  float dist = length(u_camPos - v_worldPos);
  vec3 computeNormal = terrainData.rgb;
  mat3 tbn = getTBN(computeNormal);

  float flatness = dot0(computeNormal, radialDir);
  float slope = 1.f - flatness;

  float grassWeight = smoothstep(0.f, u_grassThreshold, height);
  float snowWeight = smoothstep(u_mountainThreshold, 1.f, height);
  float slopeWeight = max(snowWeight, smoothstep(0.15f, 0.3f, slope)); // Less grass for snow
  float weights[4] = float[4](1.f, grassWeight, slopeWeight, snowWeight);

  Material surfaceMaterial = materialInit;

  for (int i = 0; i < 4; i++)
    if (weights[i] > 0.f)
      materialMix(surfaceMaterial, materialUnpack(uv, i, tbn), weights[i]);

  // surfaceMaterial.normal = normalize(surfaceMaterial.normal);
  surfaceMaterial.normal = computeNormal;

  // HACK: Refraction scattering for the water pass
  float waterScale = u_heightScale * u_heightScaleScale;
  float waterRadius = u_planetRadius + waterScale;
  float waterHeight = waterRadius - length(v_worldPos);
  float waterStep = step(-0.f, waterHeight);
  float waterDepth = exp(waterHeight * -0.0005f) * waterStep;

  surfaceMaterial.albedo = mix(surfaceMaterial.albedo, vec3(0.f, 0.4f, 0.4f), waterDepth);

//   vec3 radiance = u_lightColor * u_lightIntensity;
//   vec3 V = normalize(u_camPos - v_worldPos);
//   vec3 L = u_lightDir;
//   vec3 H = normalize(V + L);
//   float geomSmith = GeometrySmith_Direct(surfaceMaterial.roughness, surfaceMaterial.normal, V, L);

//   float ambient = 0.1f;
//   vec3 brdf = BRDF_CookTorrance(surfaceMaterial, V, L, H, geomSmith);
//   vec3 Lo = brdf * radiance * dot0(surfaceMaterial.normal, L);
//   vec3 color = Lo + surfaceMaterial.emissivity;
//   color /= color + 1.f;

  float diffuse = dot0(surfaceMaterial.normal, u_lightDir);
  float ambient = 0.01f;
  vec3 color = (diffuse + ambient) * surfaceMaterial.albedo * u_lightColor;

  // color = vec3(d);
  FragColor0 = vec4(color, 1.f);
  FragColor1 = vec4(color, 1.f);
}

