#include "common.glsl"

struct AtmosphereContext {
  vec3 originalColor;
  vec3 lightDir;
  vec3 scatteringCoefficientsR;
  vec3 scatteringCoefficientsM;
  vec3 planetCenter;
  float lightIntensity;
  float atmosphereRadius;
  float densityFalloffR;
  float densityFalloffM;
  float planetRadius;
  int scatteringPoints;
};
AtmosphereContext g_atmosphere = AtmosphereContext(
  vec3(0.f),
  vec3(0.f),
  vec3(0.f),
  vec3(0.f),
  vec3(0.f),
  0.f,
  0.f,
  0.f,
  0.f,
  0.f,
  0
);

float _Atmosphere_RayleighPhase(vec3 rayDir) {
  float cosAngle = dot(rayDir, g_atmosphere.lightDir);
  return (3.f / (16.f * PI)) * (1.f + cosAngle * cosAngle);
}

float _Atmosphere_MiePhase(vec3 rayDir) {
  const float g = 0.76f;
  const float gg = g * g;
  float cosAngle = dot(rayDir, g_atmosphere.lightDir);

  return ((1.f - gg) / pow(1.f + gg - 2.f * g * cosAngle, 1.5f));
}

float _Atmosphere_densityAtPoint(vec3 densitySamplePoint, float scale) {
  float heightAboveSurface = length(densitySamplePoint - g_atmosphere.planetCenter) - g_atmosphere.planetRadius;
  float heightNorm = heightAboveSurface / (g_atmosphere.atmosphereRadius - g_atmosphere.planetRadius);
  float density = exp(-heightNorm * scale);

  return density;
}

vec2 _Atmosphere_opticalDepthTrace(Ray ray, float rayLength, int points) {
  float stepSize = rayLength / (points - 1);
  float densityR = 0.f;
  float densityM = 0.f;

  for (int i = 0; i < points; i++) {
    vec2 p = ray.origin.xy + ray.dir.xy * stepSize;
    densityR += _Atmosphere_densityAtPoint(vec3(p, 0.f), g_atmosphere.densityFalloffR) * stepSize;
    densityM += _Atmosphere_densityAtPoint(vec3(p, 0.f), g_atmosphere.densityFalloffM) * stepSize;
  }

  return vec2(densityR, densityM);
}

vec2 _Atmosphere_opticalDepthLookUp(vec3 p, vec3 dir, sampler2D texOpticalDepth) {
  float height = length(g_atmosphere.planetCenter - p) - g_atmosphere.planetRadius;
  float heightNorm = height / (g_atmosphere.atmosphereRadius - g_atmosphere.planetRadius);
  float angleNorm = dot(normalize(p - g_atmosphere.planetCenter), g_atmosphere.lightDir) * 0.5f + 0.5f;
  vec2 uv = vec2(angleNorm, heightNorm);

  return texture(texOpticalDepth, uv).rg;
}

vec3 _Atmosphere_calcLight(Ray ray, float rayLength, sampler2D texOpticalDepth) {
  vec3 totalR = vec3(0.f);
  vec3 totalM = vec3(0.f);

  float viewRayOpticalDepthR = 0.f;
  float viewRayOpticalDepthM = 0.f;

  float stepSize = rayLength / float(g_atmosphere.scatteringPoints - 1);

  for (int i = 0; i < g_atmosphere.scatteringPoints; i++) {
    vec3 p = ray.origin + ray.dir * i * stepSize;

    float densityR = _Atmosphere_densityAtPoint(p, g_atmosphere.densityFalloffR) * stepSize;
    float densityM = _Atmosphere_densityAtPoint(p, g_atmosphere.densityFalloffM) * stepSize;

    viewRayOpticalDepthR += densityR;
    viewRayOpticalDepthM += densityM;

    vec2 sunRayOpticalDepth = _Atmosphere_opticalDepthLookUp(p, ray.dir, texOpticalDepth);

    vec3 transmittance = exp(
      -(sunRayOpticalDepth.r + viewRayOpticalDepthR) * g_atmosphere.scatteringCoefficientsR +
      -(sunRayOpticalDepth.g + viewRayOpticalDepthM) * g_atmosphere.scatteringCoefficientsM
    );

    totalR += densityR * transmittance;
    totalM += densityM * transmittance;
  }

  vec3 rayleigh = totalR * g_atmosphere.scatteringCoefficientsR * _Atmosphere_RayleighPhase(ray.dir);
  vec3 mie = totalM * g_atmosphere.scatteringCoefficientsM * _Atmosphere_MiePhase(ray.dir);

  vec3 atmosphereColor = (rayleigh + mie) * g_atmosphere.lightIntensity;

  vec3 originalColorTransmittance = exp(-(
    viewRayOpticalDepthR * g_atmosphere.scatteringCoefficientsR +
    viewRayOpticalDepthM * g_atmosphere.scatteringCoefficientsM
  ));

  return g_atmosphere.originalColor * originalColorTransmittance + atmosphereColor;
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

