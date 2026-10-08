#version 460 core

#include "../common.glsl"
#include "terrain.glsl"

out vec4 FragColor;

in vec3 v_worldPos;
in vec3 v_normal;
in vec3 v_viewVec;
in vec2 v_uv;
in flat int v_face;

uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform float u_planetRadius;
uniform float u_heightScale;
uniform float u_foamSharpness;
uniform float u_lightIntensity;
uniform float u_lightFocus;

layout(binding = 0) uniform sampler2D u_texDisplacement;
layout(binding = 1) uniform sampler2D u_texDerivatives;
layout(binding = 2) uniform sampler2D u_texTurbulence;

vec3 getWaveNormal(vec2 uv) {
  vec4 derivatives = texture(u_texDerivatives, uv);
  vec2 slope = vec2(derivatives.x / (1.f + derivatives.z), derivatives.y / (1.f + derivatives.w));
  vec3 waveNormal = normalize(vec3(-slope.x, 1.f, -slope.y));

  return waveNormal;
}

vec3 getTriplanarNormal(vec3 pos, vec3 normal, float scale) {
  vec2 uvX = pos.zy * scale;
  vec2 uvY = pos.xz * scale;
  vec2 uvZ = pos.xy * scale;

  vec3 nx = getWaveNormal(uvX) * 2.f - 1.f;
  vec3 ny = getWaveNormal(uvY) * 2.f - 1.f;
  vec3 nz = getWaveNormal(uvZ) * 2.f - 1.f;

  vec3 blend = abs(normal);
  blend = pow(blend, vec3(4.f));
  blend /= dot(blend, vec3(1.f));

  return normalize(nx * blend.x + ny * blend.y + nz * blend.z);
}

void main() {
  float viewVecLen = length(v_viewVec);
  float camHeight = viewVecLen;

  // vec3 normal = getTriplanarNormal(v_worldPos, v_normal, 0.0001f);
  vec3 normal = getWaveNormal(v_uv);
  vec3 viewDir = v_viewVec / viewVecLen;
  vec3 reflDir = reflect(-viewDir, normal);
  vec3 halfwayDir = normalize(u_lightDir + viewDir);

  float VdotN = dot0(viewDir, normal);
  float LdotN = dot0(u_lightDir, normal);
  float NdotH = dot0(normal, halfwayDir);
  float ambient = 0.1f;

  float scatter = pow(dot0(viewDir, -u_lightDir), 3.f) * (2.f - VdotN);
  vec3 scatterCol = vec3(0.f, 0.4f, 0.4f) * scatter * u_lightIntensity;

  vec3 waterBase = COLOR_SHALLOW;
  vec3 diffuseCol = waterBase * (LdotN + ambient);

  float jacobian = texture(u_texTurbulence, v_uv).r;
  float foam = 1.f - smoothstep(0.f, 1.f, jacobian * u_foamSharpness);
  foam *= exp(-camHeight * 1e-4f);

  float specAmount = pow(NdotH, u_lightFocus);
  vec3 specularCol = u_lightColor * specAmount * u_lightIntensity;

  vec3 finalColor = diffuseCol + foam + specularCol;

  FragColor = vec4(finalColor, 1.f);
}

