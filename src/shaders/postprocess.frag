#version 460

#include "terrain/common.glsl"

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
uniform float u_sunIntensity;
uniform float u_heightScale;
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
uniform float u_planetRadius;
uniform float u_atmosphereScale;
uniform float u_time;
uniform bool u_enable;

const vec3 globalUP = vec3(0.f, 1.f, 0.f);

float linearizeDepth(float depth) {
  float z = depth * 2.f - 1.f;
  float n = u_camNear;
  float f = u_camFar;

  return (2.f * n * f) / (f + n - z * (f - n));
}

void main() {
  vec3 color = texture(u_texTerrainColor, v_uv).rgb;
  if (!u_enable) {
    FragColor = vec4(color, 1.f);
    return;
  }

  color = Tonemap_ACES(color);

  FragColor = vec4(color, 1.f);
}

