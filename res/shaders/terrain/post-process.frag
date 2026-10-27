#version 460 core

#include "../common.glsl"

out vec4 FragColor;

in vec3 v_viewVec;
in vec2 v_uv;

layout(binding = 0) uniform sampler2D u_texScreenColor;
layout(binding = 1) uniform sampler2D u_texScreenDepth;
layout(binding = 2) uniform sampler2D u_texWaterHDR;
layout(binding = 3) uniform sampler2D u_texOpticalDepth;
layout(binding = 4) uniform sampler2D u_texScreenBlurredWaterSpecular;

uniform vec3 u_lightColor;
uniform vec3 u_lightDir;
uniform vec3 u_camPos;
uniform vec3 u_camForward;
uniform vec3 u_planetCenter;
uniform vec3 u_scatteringCoefficients;
uniform float u_lightIntensity;
uniform float u_lightFocus;
uniform float u_camNear;
uniform float u_camFar;
uniform float u_planetRadius;
uniform float u_densityFalloff;
uniform float u_atmosphereRadius;
uniform int u_scatteringPoints;

const float exposure = 0.25f;
const float gamma = 2.2f;

float densityAtPoint(vec3 densitySamplePoint) {
  float heightAboveSurface = length(densitySamplePoint - u_planetCenter) - u_planetRadius;
  float heightNorm = heightAboveSurface / (u_atmosphereRadius - u_planetRadius);
  float localDensity = exp(-heightNorm * u_densityFalloff) * (1.f - heightNorm);

  return localDensity;
}

float opticalDepth(vec3 p) {
  float height = length(u_planetCenter - p) - u_planetRadius;
  float heightNorm = height / (u_atmosphereRadius - u_planetRadius);
  float angleNorm = dot(normalize(p - u_planetCenter), u_lightDir) * 0.5f + 0.5f;
  vec2 uv = vec2(angleNorm, heightNorm);

  return texture(u_texOpticalDepth, uv).r;
}

vec3 calcLight(Ray ray, float rayLength, vec3 originalColor) {
  vec3 inScatteringPoint = ray.origin;
  vec3 inScatteredLight = vec3(0.f);
  float stepSize = rayLength / float(u_scatteringPoints - 1);
  float viewRayOpticalDepth = 0.f;

  for (int i = 0; i < u_scatteringPoints; i++) {
    float sunRayOpticalDepth = opticalDepth(inScatteringPoint);
    float localDensity = densityAtPoint(inScatteringPoint);
    viewRayOpticalDepth += localDensity * stepSize;
    vec3 transmittance = exp(-(sunRayOpticalDepth + viewRayOpticalDepth) * u_scatteringCoefficients);

    inScatteredLight += localDensity * transmittance * u_scatteringCoefficients * stepSize;
    inScatteringPoint += ray.dir * stepSize;
  }

  vec3 originalColorTransmittance = exp(-viewRayOpticalDepth * u_scatteringCoefficients);

  return originalColor * originalColorTransmittance + inScatteredLight * u_lightIntensity;
}

void main() {
  vec3 color = texture(u_texScreenColor, v_uv).rgb;
  vec3 colorWaterHDR = texture(u_texWaterHDR, v_uv).rgb;
  vec3 colorWaterBlurredSpecular = texture(u_texScreenBlurredWaterSpecular, v_uv).rgb;
  vec3 bloom = colorWaterHDR + colorWaterBlurredSpecular;
  bloom = vec3(1.f) - exp(-bloom * exposure);
  vec3 surfaceColor = color + bloom;

  vec3 finalColor = surfaceColor;

  // Ray ray = Ray(u_camPos, normalize(v_viewVec));
  // RayHitInfo hitInfo = raySphere(u_planetCenter, u_atmosphereRadius, ray);

  // float logDepth = texture(u_texScreenDepth, v_uv).r;
  // float eyeDepth = logDepthToLinearDepth(logDepth, u_camFar);
  // float sceneDepth = eyeDepth / dot(ray.dir, u_camForward);

  // float dstToAtmosphere = hitInfo.distToSphere;
  // float dstThroughAtmosphere = min(hitInfo.distThroughSphere, sceneDepth - dstToAtmosphere);

  // if (dstThroughAtmosphere > 0.f) {
  //   ray.origin += ray.dir * dstToAtmosphere;
  //   finalColor = calcLight(ray, dstThroughAtmosphere, finalColor);
  // }

  // if (sceneDepth > u_camFar) {
  //   float sunDot = max(dot(ray.dir, u_lightDir), 0.f);
  //   float sun = pow(sunDot, u_lightFocus) * u_lightIntensity;

  //   finalColor = sun * u_lightColor;
  // }

  finalColor = pow(finalColor, vec3(1.f / gamma));

  FragColor = vec4(finalColor, 1.f);
}

