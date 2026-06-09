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
uniform float u_time;

const vec3 globalUP = vec3(0.f, 1.f, 0.f);
const vec3 zenithColorLinear  = vec3(0.00304, 0.01938, 0.67954); // #0A26D7 -> Linear (Exact)
const vec3 horizonColorLinear = vec3(0.23455, 1.00000, 0.98225); // #85FFFD -> Linear (Exact)

float linearizeDepth(float depth) {
  float z = depth * 2.f - 1.f;
  float n = u_camNear;
  float f = u_camFar;

  return (2.f * n * f) / (f + n - z * (f - n));
}

vec3 getSkyColor(vec3 normal) {

  float horizonT = smoothstep(-0.66f, 0.5f, abs(normal.y));
  vec3 skyBase = mix(horizonColorLinear, zenithColorLinear, horizonT);
  float sun = pow(max(0.f, dot(normal, u_lightDir)), u_sunFocus) * u_sunIntensity;

  return skyBase + sun;
}

float getWaterDist(Ray ray) {
  if (ray.dir.y >= -1e-4f)
    return 1e20f;

  float t = (WATER_HEIGHT * u_heightScale - ray.origin.y) / ray.dir.y;

  return t > 0.0f ? t : 1e20f;
}

vec3 getWaterNormal(vec3 pos, float time) {
  vec2 uv = pos.xz * 0.1f;
  vec2 totalSlope = vec2(0.f);
  time *= 0.25f;

  float amp = 1.f;
  float freq = 1.f;
  float speed = 0.5f;

  for (int i = 0; i < 3; i++) {
    vec2 motion = vec2(1.f, 0.5f) * time * speed;
    vec3 n = noised(uv * freq + motion);

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
    float foamNoise = noised(waterPos.xz + u_time * 0.25f).x;
    float foamMask = smoothstep(0.5f, 0.f, waterDepth + foamNoise);

    vec3 sunRefl = reflect(u_lightDir, normal);
    float sunSpec = pow(max(0.f, dot(ray.dir, sunRefl)), 128.f) * u_sunIntensity;

    vec2 distortUV = v_uv + normal.xz * 0.02f * saturate(waterDepth);
    vec3 groundColor = texture(u_texTerrainColor, distortUV).rgb;

    float density = 0.5f;
    float murk = 1.f - exp(-waterDepth * density);
    vec3 refrColor = mix(groundColor, WATER_SHORE_COLOR, exp(-waterDepth * 0.5f));
    vec3 volumeColor = mix(refrColor, WATER_COLOR, murk);

    color = volumeColor;
    color += sunSpec * u_lightColor * 0.95f;
    color = mix(color, WATER_FOAM_COLOR, foamMask);

    dist = t_water;
  }

  if (depth >= 0.9999f) {
    float atmosphereHeight = 8000.f;
    float zenithAngle = max(0.03f, ray.dir.y);

    color = vec3(0.f);
    dist = atmosphereHeight / zenithAngle;
  }

  float sunDot = dot(ray.dir, u_lightDir);
  float rayleighPhase = PhaseRayleigh(sunDot);
  float miePhase = PhaseMie(sunDot, 0.995f);

  vec3 scattering = C_RAYLEIGH * rayleighPhase + C_MIE * miePhase;
  vec3 extinction =  C_RAYLEIGH + C_MIE;
  vec3 transmittance = exp(-dist * extinction);
  vec3 inScatterFactor = (1.f - transmittance) / extinction;
  vec3 scatteredLight = u_sunIntensity * scattering * inScatterFactor;

  color = color * transmittance + scatteredLight;

  color = Tonemap_ACES(color);
  color = pow(color, vec3(1.f / 2.2f));

  FragColor = vec4(color, 1.f);
}

