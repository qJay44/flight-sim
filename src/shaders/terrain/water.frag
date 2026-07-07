#version 460 core

#include "common.glsl"

out vec4 FragColor;

in vec3 v_sphereDir;
in vec3 v_worldPos;
in vec2 v_uv;

uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform vec3 u_camPos;
uniform float u_planetRadius;
uniform float u_heightScale;
uniform float u_seaThreshold;
uniform float u_waveScale;

layout(binding = 1) uniform sampler2D u_texWaveMap;

vec3 getTriplanarNormal(vec3 pos, vec3 normal, float scale) {
  vec2 uvX = pos.zy * scale;
  vec2 uvY = pos.xz * scale;
  vec2 uvZ = pos.xy * scale;

  vec3 nx = texture(u_texWaveMap, uvX).rgb * 2.f - 1.f;
  vec3 ny = texture(u_texWaveMap, uvY).rgb * 2.f - 1.f;
  vec3 nz = texture(u_texWaveMap, uvZ).rgb * 2.f - 1.f;

  vec3 blend = abs(normal);
  blend = pow(blend, vec3(4.f));
  blend /= dot(blend, vec3(1.f));

  return normalize(nx * blend.x + ny * blend.y + nz * blend.z);
}

vec3 getSkyColor(vec3 rayDir) {
  // 1. Horizon Gradient (White/Light Blue at horizon, Deep Blue up top)
  float horizonFactor = smoothstep(-0.1, 0.3, rayDir.y);
  vec3 zenithColor = vec3(0.05, 0.1, 0.3); // Dark Space Blue
  vec3 horizonColor = vec3(0.6, 0.7, 0.9); // Pale Haze
  vec3 skyBase = mix(horizonColor, zenithColor, horizonFactor);

  // 2. Sun Halo (Bright glow around the sun)
  float sunSpot = max(dot(rayDir, u_lightDir), 0.0);
  float sunHalo = pow(sunSpot, 200.0); // Sharp sun disk

  return skyBase + u_lightColor * sunHalo;
}

void main() {
  float height = u_planetRadius / u_heightScale;

  vec3 geometricNormal = normalize(v_sphereDir);
  vec3 waveNormal = getTriplanarNormal(v_worldPos, geometricNormal, u_waveScale);
  vec3 finalNormal = normalize(geometricNormal + waveNormal * 0.5f); // Perturbed Normal, 0.5 is wave strength

  vec3 viewDir = normalize(u_camPos-v_worldPos);
  vec3 halfwayDir = normalize(u_lightDir + viewDir);
  vec3 reflDir = reflect(-viewDir, finalNormal);
  vec3 reflColor = getSkyColor(reflDir);

  float NdotH = max(dot(finalNormal, halfwayDir), 0.f);
  float spec = pow(NdotH, 128.f);

  float NdotV = max(dot(geometricNormal, viewDir), 0.f);
  float fresnel = pow(1.f - NdotV, 5.f);

  // 0.02 - Water is 98% transparent looking down
  // 0.95 - Water is 95% opaque at the horison
  fresnel = clamp(fresnel + 0.02f, 0.f, 0.95f);

  vec4 surfaceColor = vec4(COLOR_SHALLOW, fresnel);
  vec3 finalColor = surfaceColor.rgb + u_lightColor * spec;

  FragColor = vec4(finalColor, surfaceColor.a + spec);
}

