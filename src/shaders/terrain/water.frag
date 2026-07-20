#version 460 core

#include "common.glsl"

out vec4 FragColor;

in vec3 v_sphereDir;
in vec3 v_worldPos;
in vec2 v_uv;
in flat int v_id;

uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform vec3 u_camPos;
uniform float u_planetRadius;
uniform float u_heightScale;
uniform float u_seaThreshold;
uniform float u_waveScale;
uniform float u_foamSharpness;
uniform float u_sunIntensiy;

layout(binding = 0) uniform sampler2D u_texArray;
layout(binding = 1) uniform sampler2D u_texDisplacement;
layout(binding = 2) uniform sampler2D u_texDerivatives;
layout(binding = 3) uniform sampler2D u_texTurbulence;

layout(std140, binding = 0) uniform NodesDataBlock {
  NodeData nodesData[MAX_NODES];
};

vec3 getNormal(vec2 uv) {
  vec4 derivatives = texture(u_texDerivatives, uv);
  vec2 slope = vec2(derivatives.x / (1.f + derivatives.z), derivatives.y / (1.f + derivatives.w));
  vec3 normal = normalize(vec3(-slope.x, 1.f, -slope.y));

  return normal;
}

vec3 getTriplanarNormal(vec3 pos, vec3 normal, float scale) {
  vec2 uvX = pos.zy * scale;
  vec2 uvY = pos.xz * scale;
  vec2 uvZ = pos.xy * scale;

  vec3 nx = getNormal(uvX) * 2.f - 1.f;
  vec3 ny = getNormal(uvY) * 2.f - 1.f;
  vec3 nz = getNormal(uvZ) * 2.f - 1.f;

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
  NodeData node = nodesData[v_id];

  vec3 viewVec = u_camPos - v_worldPos;
  float height = u_planetRadius / u_heightScale;
  float camHeight = length(viewVec);

  vec3 geometricNormal = normalize(v_sphereDir);
  vec3 waveNormal = getTriplanarNormal(v_worldPos, geometricNormal, 1.f / node.extents);
  vec3 finalNormal = normalize(geometricNormal + waveNormal * 0.5f); // Perturbed Normal, 0.5 is wave strength

  vec3 viewDir = viewVec / camHeight;
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

  float jacobian = texture(u_texTurbulence, v_uv).r;
  float foam = 1.f - smoothstep(0.f, 1.f, jacobian * u_foamSharpness);
  foam *= exp(-camHeight * 1e-4f);

  surfaceColor += foam;

  vec3 finalColor = surfaceColor.rgb + u_lightColor * spec * u_sunIntensiy;

  FragColor = vec4(finalColor, surfaceColor.a + spec);
}

