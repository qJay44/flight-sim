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
uniform float u_time;
uniform float u_waterDensity;
uniform float u_refractionDistortScale;
uniform float u_ambient;
uniform float u_diffusePowScale;
uniform float u_specularPowScale;
uniform float u_triplanarSharpness;
uniform float u_triplanarScale;

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

vec3 derivativesToNormal(vec4 derivatives) {
  vec2 slope = vec2(derivatives.x / (1.f + derivatives.z), derivatives.y / (1.f + derivatives.w));
  vec3 normal = normalize(vec3(-slope.x, -slope.y, 1.f));

  return normal;
}

vec3 getWaveNormal() {
  vec3 absNormal = abs(v_sphereDir);
  vec3 weights = pow(absNormal, vec3(u_triplanarSharpness));
  weights /= (weights.x + weights.y + weights.z);

  vec3 scaledWorldPos = v_worldPos * u_triplanarScale;
  vec4 derivativesX = texture(u_texDerivatives, scaledWorldPos.zy);
  vec4 derivativesY = texture(u_texDerivatives, scaledWorldPos.xz);
  vec4 derivativesZ = texture(u_texDerivatives, scaledWorldPos.xy);

  vec3 tangentNormalX = derivativesToNormal(derivativesX);
  vec3 tangentNormalY = derivativesToNormal(derivativesY);
  vec3 tangentNormalZ = derivativesToNormal(derivativesZ);

  vec3 axisSign = sign(v_sphereDir);
  tangentNormalX.xy *= axisSign.x;
  tangentNormalY.xy *= axisSign.y;
  tangentNormalZ.xy *= axisSign.z;

  vec3 worldX = vec3(v_sphereDir.x, tangentNormalX.yx);
  vec3 worldY = vec3(tangentNormalY.x, v_sphereDir.y, tangentNormalY.y);
  vec3 worldZ = vec3(tangentNormalZ.xy, v_sphereDir.z);

  return normalize(worldX * weights.x + worldY * weights.y + worldZ * weights.z);
}

float getJacobian() {
  vec3 absNormal = abs(v_sphereDir);
  vec3 weights = pow(absNormal, vec3(u_triplanarSharpness));
  weights /= (weights.x + weights.y + weights.z);

  vec3 scaledWorldPos = v_worldPos * u_triplanarScale;
  float jacobianX = texture(u_texTurbulence, scaledWorldPos.zy).r;
  float jacobianY = texture(u_texTurbulence, scaledWorldPos.xz).r;
  float jacobianZ = texture(u_texTurbulence, scaledWorldPos.xy).r;

  return jacobianX * weights.x + jacobianY * weights.y + jacobianZ * weights.z;
}

vec3 getWaterReflection(vec2 screenUV, vec3 normal, vec3 viewDir) {
  vec3 reflNormal = reflect(-viewDir, normal);
  vec3 ambient = u_oceanDeepColor * 0.1f;
  return max(ambient, u_oceanDeepColor * dot0(reflNormal, u_lightDir)); // same as ocean color...
}

void main() {
  vec2 screenUV = gl_FragCoord.xy / textureSize(u_texLandColor, 0).xy;
  vec3 viewDir = normalize(u_camPos - v_worldPos);
  float waterScale = u_heightScale * u_heightScaleScale;
  float waterRadius = u_planetRadius + waterScale;

  float dist = max0(v_viewDist);
  float distScale = exp(-dist / u_heightScale);

  vec3 normal = mix(getWaveNormal(), v_sphereDir, 1.f - distScale);
  vec3 halfwayDir = normalize(u_lightDir + viewDir);
  distScale = 1.f - distScale;

  vec2 distortedUV = screenUV + normal.xz * u_refractionDistortScale;
  vec4 landDataDepth = texture(u_texLandDepth, distortedUV);
  vec3 seabedColor = texture(u_texLandColor, distortedUV).rgb;

  float landDepth = landDataDepth.r;
  float landDepthLinear = logDepthToLinearDepth(landDepth, u_camFar);

  float VdotN = dot0(viewDir, normal);
  float NdotL = dot0(normal, u_lightDir);
  float NdotH = dot0(normal, halfwayDir);

  float jacobian = getJacobian();
  float foam = 1.f - smoothstep(0.f, 1.f, jacobian * u_foamSharpness);
  foam *= pow(1.f - gl_FragCoord.z, 5.f);

  float depthT = smoothstep(0.f, waterRadius, landDepthLinear * 0.5f);
  depthT = exp(-depthT * u_waterDensity);

  vec3 waterBase = mix(u_oceanDeepColor, seabedColor, depthT);

  float f0 = 0.02f;
  float fresnel = f0 + (1.f - f0) * pow(1.f - VdotN, 5.f);

  float diffuse = pow(NdotL * 0.4f + 0.6f, u_diffusePowScale);
  float specular = pow(NdotH, u_specularPowScale) * u_lightIntensity * distScale;

  vec3 specularColor = specular * u_lightColor;
  vec3 reflColor = getWaterReflection(screenUV, normal, viewDir);
  reflColor *= distScale;

  vec3 finalColor = NdotL * u_lightColor * waterBase;
  finalColor = mix(finalColor, reflColor, fresnel);
  finalColor += specularColor;
  finalColor = mix(finalColor, finalColor + vec3(1.f), foam);

  vec3 finalColorHDR = specularColor * 10.f;
  FragColor = vec4(finalColor, 1.f);
  FragColorHDR = vec4(finalColorHDR, 1.f);
}

