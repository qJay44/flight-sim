#version 460 core

#include "../common.glsl"
#include "terrain.glsl"

out vec4 FragColor;

in vec3 v_sphereDir;
in float v_viewDist;
in flat Ray v_ray;

uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform float u_planetRadius;
uniform float u_heightScale;
uniform float u_foamSharpness;
uniform float u_lightIntensity;
uniform float u_lightFocus;
uniform float u_tileScale;

layout(binding = 0) uniform sampler2D u_texDisplacement;
layout(binding = 1) uniform sampler2D u_texDerivatives;
layout(binding = 2) uniform sampler2D u_texTurbulence;

vec3 getWaveNormal(vec2 uv) {
  vec4 derivatives = texture(u_texDerivatives, uv);
  vec2 slope = vec2(derivatives.x / (1.f + derivatives.z), derivatives.y / (1.f + derivatives.w));
  vec3 waveNormal = normalize(vec3(-slope.x, 1.f, -slope.y));

  return waveNormal;
}

void main() {
  vec3 viewDir = v_ray.dir;

  vec2 equatorUV = vec2(atan(v_sphereDir.z, v_sphereDir.x), asin(v_sphereDir.y)) * u_tileScale;
  vec2 polarUV = v_sphereDir.xz * u_tileScale;
  float poleBlend = smoothstep(0.6f, 0.8f, abs(v_sphereDir.y));

  vec3 normalEquator = getWaveNormal(equatorUV);
  vec3 normalPolar = getWaveNormal(polarUV);
  vec3 normal = mix(normalEquator, normalPolar, poleBlend);
  vec3 reflDir = reflect(-viewDir, normal);
  vec3 halfwayDir = normalize(u_lightDir + viewDir);

  float VdotN = dot0(viewDir, normal);
  float LdotN = dot0(u_lightDir, normal);
  float NdotH = dot0(normal, halfwayDir);
  float ambient = 0.1f;

  float scatter = pow(dot0(viewDir, -u_lightDir), 2.f) * (1.f - VdotN);
  vec3 scatterCol = vec3(0.f, 0.4f, 0.4f) * scatter * u_lightIntensity;

  vec3 waterBase = vec3(0.02f, 0.1f, 0.2f);
  vec3 diffuseCol = waterBase * (LdotN + ambient);

  float jacobianEquator = texture(u_texTurbulence, equatorUV).r;
  float jacobianPolar = texture(u_texTurbulence, polarUV).r;
  float jacobian = mix(jacobianEquator, jacobianPolar, poleBlend);
  float foam = 1.f - smoothstep(0.f, 1.f, jacobian * u_foamSharpness);
  foam *= exp(-v_viewDist * 1e-4f);

  float specAmount = pow(NdotH, u_lightFocus);
  vec3 specularCol = u_lightColor * specAmount * u_lightIntensity;

  vec3 finalColor = diffuseCol + foam + specularCol;

  FragColor = vec4(finalColor, 1.f);
}

