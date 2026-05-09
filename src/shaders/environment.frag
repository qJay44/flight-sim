#version 460 core

out vec4 FragColor;

in vec2 v_uv;

uniform vec3 u_camPos;
uniform vec3 u_lightPos;
uniform vec3 u_skyHorizonColor;
uniform vec3 u_skyZenithColor;
uniform vec3 u_groundColor;
uniform vec3 u_sunDir;
uniform vec3 u_sunColor;
uniform float u_sunFocus;
uniform float u_sunIntensity;
uniform mat4 u_camInv;

void main() {
  vec2 ndc = v_uv * 2.f - 1.f;
  vec4 clipPos = vec4(ndc, -1.f, 1.f);
  vec4 worldPos = u_camInv * clipPos;
  worldPos /= worldPos.w;

  vec3 rayDir = normalize(worldPos.xyz - u_camPos);
  vec3 lightDir = -u_sunDir;

  float skyGradientT = pow(smoothstep(0.f, 0.4f, rayDir.y), 0.35f);
  vec3 skyGradient = mix(u_skyHorizonColor, u_skyZenithColor, skyGradientT);
  float sun = pow(max(0.f, dot(rayDir, -lightDir)), u_sunFocus) * u_sunIntensity;

  float groundToSkyT = smoothstep(-0.01f, 0.f, rayDir.y);
  vec3 col = mix(u_groundColor, skyGradient, groundToSkyT) + sun * u_sunColor;

  FragColor = vec4(col, 1.f);
}

