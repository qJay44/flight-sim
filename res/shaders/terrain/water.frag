#version 460 core

#include "../common.glsl"
#include "terrain.glsl"

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragColorHDR;

in vec3 v_worldPos;
in vec3 v_sphereDir;
in float v_viewDist;
in flat Ray v_ray;

uniform mat4 u_proj;
uniform mat4 u_localView;
uniform mat4 u_localTranslation;
uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform vec3 u_planetCenter;
uniform vec3 u_camPos;
uniform vec3 u_oceanDeepColor;
uniform vec3 u_oceanShallowColor;
uniform float u_camNear;
uniform float u_camFar;
uniform float u_planetRadius;
uniform float u_heightScale;
uniform float u_heightScaleScale;
uniform float u_foamSharpness;
uniform float u_lightIntensity;
uniform float u_lightFocus;
uniform float u_tileScale;
uniform float u_time;
uniform float u_waterDensity;
uniform float u_refractionDistortScale;
uniform float u_normalToScatterScale;
uniform float u_ambient;
uniform float u_diffusePowScale;
uniform float u_specularPowScale;

layout(binding = 0) uniform sampler2D u_texDisplacement;
layout(binding = 1) uniform sampler2D u_texDerivatives;
layout(binding = 2) uniform sampler2D u_texTurbulence;
layout(binding = 3) uniform sampler2D u_texLandColor;
layout(binding = 4) uniform sampler2D u_texLandDepth;

mat3 getTBN(vec3 localNormal, vec3 worldPos, vec2 uv) {
  vec3 q1 = dFdx(worldPos);
  vec3 q2 = dFdy(worldPos);
  vec2 st1 = dFdx(uv);
  vec2 st2 = dFdy(uv);

  vec3 N = localNormal;
  vec3 T = normalize(q1 * st2.t - q2 * st1.t);
  vec3 B = -normalize(cross(N, T));

  return mat3(T, B, N);
}

vec3 getWaveNormal(vec2 equatorUV, vec2 polarUV, float poleBlend) {
  vec4 derivatives = texture(u_texDerivatives, equatorUV);
  vec2 slope = vec2(derivatives.x / (1.f + derivatives.z), derivatives.y / (1.f + derivatives.w));
  vec3 tangentNormal = normalize(vec3(-slope.x, -slope.y, 1.f));
  vec3 normalEquator = normalize(getTBN(v_sphereDir, v_worldPos, equatorUV) * tangentNormal);

  derivatives = texture(u_texDerivatives, polarUV);
  slope = vec2(derivatives.x / (1.f + derivatives.z), derivatives.y / (1.f + derivatives.w));
  tangentNormal = normalize(vec3(-slope.x, -slope.y, 1.f));
  vec3 normalPolar = normalize(getTBN(v_sphereDir, v_worldPos, polarUV) * tangentNormal);

  return mix(normalEquator, normalPolar, poleBlend);
}

float getJacobian(vec2 equatorUV, vec2 polarUV, float poleBlend) {
  float jacobianEquator = texture(u_texTurbulence, equatorUV).r;
  float jacobianPolar = texture(u_texTurbulence, polarUV).r;
  float jacobian = mix(jacobianEquator, jacobianPolar, poleBlend);

  return jacobian;
}

vec3 getWaterReflection(vec2 screenUV, vec3 normal, vec3 viewDir) {
  // TODO: Atmosphere reflection
  vec3 res = vec3(0.0f, 0.1f, 0.1f);

  return res;
}

void main() {
  vec2 screenUV = gl_FragCoord.xy / textureSize(u_texLandColor, 0).xy;
  vec3 viewDir = normalize(u_camPos - v_worldPos);
  float waterRadius = u_planetRadius + u_heightScale * u_heightScaleScale;
  RayHitInfo hitInfo = raySphere(u_planetCenter, waterRadius, v_ray);

  float distScale = smoothstep(0.f, u_planetRadius - u_heightScale * 9.f, max(0.f, v_viewDist));
  float distScale1 = 1.f - distScale;

  float uvScale = u_tileScale;
  vec2 equatorUV = vec2(atan(v_sphereDir.z, v_sphereDir.x), asin(v_sphereDir.y)) * uvScale;
  vec2 polarUV = v_sphereDir.xz * uvScale;
  float poleBlend = smoothstep(0.6f, 0.8f, abs(v_sphereDir.y));

  vec3 normal = getWaveNormal(equatorUV, polarUV, poleBlend);
  vec3 halfwayDir = normalize(u_lightDir + viewDir);

  normal = mix(normal, v_sphereDir, exp(-v_viewDist));

  vec2 distortedUV = screenUV + normal.xz * u_refractionDistortScale;
  vec4 landDataColor = texture(u_texLandColor, distortedUV);
  vec4 landDataDepth = texture(u_texLandDepth, distortedUV);
  vec3 seabedColor = landDataColor.rgb;

  float landDepth = landDataDepth.r;
  float waterDepth = gl_FragCoord.z;
  float landDepthLinear = linearizeDepth(landDepth, u_camNear, u_camFar);
  float waterDepthLinear = linearizeDepth(waterDepth, u_camNear, u_camFar);
  float waterThickness = landDepthLinear - waterDepthLinear;
  float absorption = exp(-max0(waterThickness) * u_waterDensity) * distScale1;

  float VdotN = dot0(viewDir, normal);
  float NdotL = dot0(normal, u_lightDir);
  float NdotH = dot0(normal, halfwayDir);

  float jacobian = getJacobian(equatorUV, polarUV, poleBlend);
  float foam = 1.f - smoothstep(0.f, 1.f, jacobian * u_foamSharpness);
  foam *= distScale1;

  vec3 scatterDir = normalize(u_lightDir + normal * u_normalToScatterScale);
  float scatter = pow(dot0(-viewDir, scatterDir), 3.f);
  vec3 scatterCol = u_oceanShallowColor * scatter * jacobian * distScale1;

  seabedColor = mix(seabedColor, u_oceanShallowColor, absorption);
  vec3 waterBase = mix(u_oceanDeepColor, seabedColor, waterDepth);

  float f0 = 0.02f;
  float fresnel = f0 + (1.f - f0) * pow(1.f - VdotN, 5.f);

  float diffuse = pow(NdotL * 0.4f + 0.6f, u_diffusePowScale);
  float specular = pow(NdotH, u_specularPowScale) * u_lightIntensity * distScale1;

  vec3 specularColor = specular * u_lightColor;
  vec3 reflColor = getWaterReflection(screenUV, normal, viewDir);
  reflColor *= distScale1;

  vec3 finalColor = (diffuse + u_ambient) * u_lightColor * waterBase;
  finalColor = mix(finalColor, reflColor, fresnel);
  finalColor += scatterCol;
  finalColor += specularColor;
  finalColor = mix(finalColor, finalColor + vec3(1.f), foam);

  vec3 finalColorHDR = specularColor * 2.f;

  FragColor = vec4(finalColor, 1.f);
  FragColorHDR = vec4(finalColorHDR, 1.f);
}

