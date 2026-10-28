#include "common.glsl"

struct AtmosphereContext {
  vec3 originalColor;
  vec3 lightDir;
  vec3 scatteringCoefficients;
  vec3 planetCenter;
  float lightIntensity;
  float atmosphereRadius;
  float densityFalloff;
  float planetRadius;
  int scatteringPoints;
};
AtmosphereContext g_atmosphere = AtmosphereContext(
  vec3(0.f),
  vec3(0.f),
  vec3(0.f),
  vec3(0.f),
  0.f,
  0.f,
  0.f,
  0.f,
  0
);

float _Atmosphere_densityAtPoint(vec3 densitySamplePoint) {
  float heightAboveSurface = length(densitySamplePoint - g_atmosphere.planetCenter) - g_atmosphere.planetRadius;
  float heightNorm = heightAboveSurface / (g_atmosphere.atmosphereRadius - g_atmosphere.planetRadius);
  float localDensity = exp(-heightNorm * g_atmosphere.densityFalloff) * (1.f - heightNorm);

  return localDensity;
}

float _Atmosphere_opticalDepth(vec3 p, sampler2D texOpticalDepth) {
  float height = length(g_atmosphere.planetCenter - p) - g_atmosphere.planetRadius;
  float heightNorm = height / (g_atmosphere.atmosphereRadius - g_atmosphere.planetRadius);
  float angleNorm = dot(normalize(p - g_atmosphere.planetCenter), g_atmosphere.lightDir) * 0.5f + 0.5f;
  vec2 uv = vec2(angleNorm, heightNorm);

  return texture(texOpticalDepth, uv).r;
}

vec3 _Atmosphere_calcLight(Ray ray, float rayLength, sampler2D texOpticalDepth) {
  vec3 inScatteringPoint = ray.origin;
  vec3 inScatteredLight = vec3(0.f);
  float stepSize = rayLength / float(g_atmosphere.scatteringPoints - 1);
  float viewRayOpticalDepth = 0.f;

  for (int i = 0; i < g_atmosphere.scatteringPoints; i++) {
    float sunRayOpticalDepth = _Atmosphere_opticalDepth(inScatteringPoint, texOpticalDepth);
    float localDensity = _Atmosphere_densityAtPoint(inScatteringPoint);
    viewRayOpticalDepth += localDensity * stepSize;
    vec3 transmittance = exp(-(sunRayOpticalDepth + viewRayOpticalDepth) * g_atmosphere.scatteringCoefficients);

    inScatteredLight += localDensity * transmittance * g_atmosphere.scatteringCoefficients * stepSize;
    inScatteringPoint += ray.dir * stepSize;
  }

  vec3 originalColorTransmittance = exp(-viewRayOpticalDepth * g_atmosphere.scatteringCoefficients);

  return g_atmosphere.originalColor * originalColorTransmittance + inScatteredLight * g_atmosphere.lightIntensity;
}

vec3 Atmosphere_getColor(Ray ray, RayHitInfo hitInfo, float sceneDepth, sampler2D texOpticalDepth) {
  float dstToAtmosphere = hitInfo.distToSphere;
  float dstThroughAtmosphere = min(hitInfo.distThroughSphere, sceneDepth - dstToAtmosphere);
  vec3 res = g_atmosphere.originalColor;

  if (dstThroughAtmosphere > 0.f) {
    ray.origin += ray.dir * dstToAtmosphere;
    res = _Atmosphere_calcLight(ray, dstThroughAtmosphere, texOpticalDepth);
  }

  return res;
}

