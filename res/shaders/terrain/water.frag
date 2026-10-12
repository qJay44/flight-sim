#version 460 core

#include "../common.glsl"
#include "terrain.glsl"

out vec4 FragColor;

in vec3 v_worldPos;
in vec3 v_sphereDir;
in vec3 v_viewVec;
in float v_viewDist;
in float v_depth;
in flat Ray v_ray;

uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform vec3 u_planetCenter;
uniform float u_planetRadius;
uniform float u_heightScale;
uniform float u_heightScaleScale;
uniform float u_foamSharpness;
uniform float u_lightIntensity;
uniform float u_lightFocus;
uniform float u_tileScale;

layout(binding = 0) uniform sampler2D u_texDisplacement;
layout(binding = 1) uniform sampler2D u_texDerivatives;
layout(binding = 2) uniform sampler2D u_texTurbulence;

mat3 getTBN(vec3 localNormal, vec2 uv) {
  vec3 q1 = dFdx(v_worldPos);
  vec3 q2 = dFdy(v_worldPos);
  vec2 st1 = dFdx(uv);
  vec2 st2 = dFdy(uv);

  vec3 N = localNormal;
  vec3 T = normalize(q1 * st2.t - q2 * st1.t);
  vec3 B = -normalize(cross(N, T));

  return mat3(T, B, N);
}

vec3 getWaveNormal(vec2 uv) {
  vec4 derivatives = texture(u_texDerivatives, uv);
  vec2 slope = vec2(derivatives.x / (1.f + derivatives.z), derivatives.y / (1.f + derivatives.w));
  vec3 tangentNormal = normalize(vec3(-slope.x, -slope.y, 1.f));
  vec3 normal = normalize(getTBN(v_sphereDir, uv) * tangentNormal);

  return normal;
}

void main() {
  vec3 viewDir = v_ray.dir;
  float waterDepth = u_heightScale * u_heightScaleScale;
  float waterRadius = u_planetRadius + waterDepth;
  float depthT = 1.f - smoothstep(0.f, waterDepth, v_depth);
  RayHitInfo hitInfo = raySphere(u_planetCenter, waterRadius, v_ray);

  vec2 equatorUV = vec2(atan(v_sphereDir.z, v_sphereDir.x), asin(v_sphereDir.y)) * u_tileScale;
  vec2 polarUV = v_sphereDir.xz * u_tileScale;
  float poleBlend = smoothstep(0.6f, 0.8f, abs(v_sphereDir.y));

  vec3 normalEquator = getWaveNormal(equatorUV);
  vec3 normalPolar = getWaveNormal(polarUV);
  vec3 normal = mix(normalEquator, normalPolar, poleBlend);
  vec3 reflDir = reflect(-viewDir, normal);
  vec3 halfwayDir = normalize(u_lightDir + viewDir);
  float distScale = smoothstep(0.f, u_planetRadius - u_heightScale * 10.f, max(0.f, v_viewDist));

  normal = mix(normal, v_sphereDir, distScale);

  float VdotN = dot0(viewDir, normal);
  float LdotN = dot0(u_lightDir, normal);
  float NdotH = dot0(normal, halfwayDir);
  float ambient = 0.1f;

  float scatter = pow(dot0(viewDir, -u_lightDir), 2.f) * (1.f - VdotN);
  vec3 scatterCol = vec3(0.f, 0.4f, 0.4f) * scatter * u_lightIntensity;

  float jacobianEquator = texture(u_texTurbulence, equatorUV).r;
  float jacobianPolar = texture(u_texTurbulence, polarUV).r;
  float jacobian = mix(jacobianEquator, jacobianPolar, poleBlend);
  float foam = 1.f - smoothstep(0.f, 1.f, jacobian * u_foamSharpness);
  foam *= 1.f - distScale;
  foam = foam * max(0.4f, LdotN);

  vec3 waterBase = mix(COLOR_OCEAN_SHALLOW, COLOR_OCEAN_DEEP, depthT);
  vec3 diffuseCol = waterBase * (LdotN + ambient);

  float specAmount = pow(NdotH, u_lightFocus);
  vec3 specularCol = u_lightColor * specAmount * u_lightIntensity;

  vec3 finalColor = diffuseCol + foam + specularCol;
  // finalColor = pow(finalColor, vec3(1.f / 2.2f));

  FragColor = vec4(finalColor, 1.f);
  // FragColor = vec4(vec3(depthT), 1.f);
}

