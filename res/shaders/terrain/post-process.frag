#version 460 core

#include "../atmosphere.glsl"

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

void main() {
  vec3 color = texture(u_texScreenColor, v_uv).rgb;
  vec3 colorWaterHDR = texture(u_texWaterHDR, v_uv).rgb;
  vec3 colorWaterBlurredSpecular = texture(u_texScreenBlurredWaterSpecular, v_uv).rgb;
  vec3 bloom = colorWaterHDR + colorWaterBlurredSpecular;
  bloom = vec3(1.f) - exp(-bloom * exposure);
  vec3 surfaceColor = color + bloom;

  vec3 finalColor = surfaceColor;

  Ray ray = Ray(u_camPos, normalize(v_viewVec));
  RayHitInfo hitInfo = raySphere(u_planetCenter, u_atmosphereRadius, ray);

  float logDepth = texture(u_texScreenDepth, v_uv).r;
  float eyeDepth = logDepthToLinearDepth(logDepth, u_camFar);
  float sceneDepth = eyeDepth / dot(ray.dir, u_camForward);

  g_atmosphere.originalColor          = finalColor;
  g_atmosphere.lightDir               = u_lightDir;
  g_atmosphere.scatteringCoefficients = u_scatteringCoefficients;
  g_atmosphere.planetCenter           = u_planetCenter;
  g_atmosphere.lightIntensity         = u_lightIntensity;
  g_atmosphere.atmosphereRadius       = u_atmosphereRadius;
  g_atmosphere.densityFalloff         = u_densityFalloff;
  g_atmosphere.planetRadius           = u_planetRadius;
  g_atmosphere.scatteringPoints       = u_scatteringPoints;

  finalColor = Atmosphere_getColor(ray, hitInfo, sceneDepth, u_texOpticalDepth);

  float sunDot = dot0(ray.dir, u_lightDir);
  float sun = pow(sunDot, u_lightFocus) * u_lightIntensity;
  finalColor += sun * u_lightColor;

  finalColor = pow(finalColor, vec3(1.f / gamma));

  FragColor = vec4(finalColor, 1.f);
}

