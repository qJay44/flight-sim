#version 460

#include "terrain/common.glsl"

#define WATER_COLOR vec3(0.0, 0.05, 0.1)
#define WATER_SHORE_COLOR vec3(0.0, 0.25, 0.25)
#define WATER_FOAM_COLOR vec3(1.f)

in vec3 v_rayDir;
in vec2 v_uv;

out vec4 FragColor;

layout (binding = 0) uniform sampler2D u_texTerrainColor;
layout (binding = 1) uniform sampler2D u_texTerrainDepth;

uniform vec3 u_camPos;
uniform vec3 u_camForward;
uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform float u_camNear;
uniform float u_camFar;
uniform float u_sunFocus;
uniform float u_sunIntensity;
uniform float u_heightScale;
uniform float u_heightmapScale;
uniform float u_appearance;
uniform float u_waterShoreScale;
uniform float u_waterRefractionScale;
uniform float u_waterRefractionDistortScale;
uniform float u_waterNormalScaleUV;
uniform float u_waterNoiseScale;
uniform float u_foamEdge0;
uniform float u_foamEdge1;
uniform float u_fogDensity;
uniform float u_fogDensityFalloff;
uniform float u_horizonThickness;
uniform float u_horizonFalloff;
uniform float u_time;

const vec3 globalUP = vec3(0.f, 1.f, 0.f);

float linearizeDepth(float depth) {
  float z = depth * 2.f - 1.f;
  float n = u_camNear;
  float f = u_camFar;

  return (2.f * n * f) / (f + n - z * (f - n));
}

float getWaterDist(Ray ray) {
  if (ray.dir.y >= -1e-4f)
    return 1e20f;

  float t = (WATER_HEIGHT * u_heightScale - ray.origin.y) / ray.dir.y;

  return t > 0.0f ? t : 1e20f;
}

vec3 getWaterNormal(vec3 pos, float time) {
  vec2 uv = pos.xz * u_waterNormalScaleUV;
  vec2 totalSlope = vec2(0.f);

  float amp = 1.f;
  float freq = 1.f;
  float speed = 0.5f;

  for (int i = 0; i < 3; i++) {
    vec2 motion = vec2(1.f, 0.5f) * time * speed;
    vec3 n = noised(uv * freq + motion) * u_waterNoiseScale;

    totalSlope -= n.yz * amp;

    float c = 0.866f;
    float s = 0.5f;
    mat2 rot = mat2(c, -s, s, c);
    uv = rot * uv;

    freq *= 2.f;
    amp *= 0.5f;
    speed *= 1.2f;
  }

  return normalize(vec3(totalSlope.x, 1.f, totalSlope.y));
}

float getExponentialDensityIntegral(Ray ray, float rayLength) {
  float falloff = u_fogDensityFalloff;

  float h0 = ray.origin.y - WATER_HEIGHT;
  float h1 = (ray.origin.y + ray.dir.y * rayLength) - WATER_HEIGHT;

  // Check if the ray is horizontal to avoid division by zero
  if (abs(ray.dir.y) < 0.0001f)
    return exp(-falloff * h0) * rayLength;

  return (exp(-falloff * h0) - exp(-falloff * h1)) / (falloff * ray.dir.y);
}

void main() {
  Ray ray = Ray(u_camPos, normalize(v_rayDir));

  float depth = texture(u_texTerrainDepth, v_uv).r;
  float t_terrain = linearizeDepth(depth) / dot(ray.dir, u_camForward);
  float t_water = getWaterDist(ray);

  vec3 color = texture(u_texTerrainColor, v_uv).rgb;
  float dist = t_terrain;

  if (t_water > 0.f && t_water < t_terrain) {
    vec3 waterPos = ray.origin + t_water * ray.dir;
    vec3 normal = getWaterNormal(waterPos, u_time);
    float waterDepth = t_terrain - t_water;

    vec3 sunRefl = reflect(u_lightDir, normal);
    float sunSpec = pow(max(0.f, dot(ray.dir, sunRefl)), 128.f) * u_sunIntensity;

    vec2 distortUV = v_uv + normal.xz * u_waterRefractionDistortScale * saturate(waterDepth);
    vec3 groundColor = texture(u_texTerrainColor, distortUV).rgb;

    float shore = exp(-waterDepth * u_waterShoreScale);
    float refr = exp(-waterDepth * u_waterRefractionScale);

    color = mix(WATER_COLOR, WATER_SHORE_COLOR, shore);
    color = mix(color, groundColor, refr);

    color += sunSpec * u_lightColor * 0.95f;

    dist = min(t_water, u_camFar);
  }

  float atmosphereHeight = 8000.f;
  float zenithAngle = max(0.03f, ray.dir.y);
  float maxAtmoDist = atmosphereHeight / zenithAngle;

  if (depth >= 0.9999f) {
    color = vec3(0.f);
    dist = maxAtmoDist;
  } else {
    dist = min(t_terrain, maxAtmoDist);
  }

  float horizonFactor = 1.f - abs(ray.dir.y);
  float horizonHaze = pow(horizonFactor, u_horizonFalloff) * u_horizonThickness;
  float opticalDepthScale = getExponentialDensityIntegral(ray, dist);
  float effectiveDist = opticalDepthScale * (u_fogDensity + horizonHaze);

  float sunDot = dot(ray.dir, u_lightDir);
  float rayleighPhase = PhaseRayleigh(sunDot);
  float miePhase = PhaseMie(sunDot, 0.995f);

  vec3 scattering = C_RAYLEIGH * rayleighPhase + C_MIE * miePhase;
  vec3 extinction =  C_RAYLEIGH + C_MIE;
  vec3 transmittance = exp(-effectiveDist * extinction);
  vec3 inScatterFactor = (1.f - transmittance) / extinction;
  vec3 scatteredLight = u_sunIntensity * scattering * inScatterFactor;

  color = color * transmittance + scatteredLight;

  color = Tonemap_ACES(color);

  FragColor = vec4(color, 1.f);
}

