#version 460 core

#include "../common.glsl"

layout(location = 0) in vec3 a_pos;

out vec3 v_worldPos;
out vec3 v_sphereDir;
out float v_viewDist;
out flat Ray v_ray;

uniform mat4 u_proj;
uniform mat4 u_pvInvLocal;
uniform mat4 u_localView;
uniform mat4 u_localTranslation;
uniform vec3 u_camPos;
uniform vec3 u_planetCenter;
uniform float u_camFar;
uniform float u_planetRadius;
uniform float u_heightScale;
uniform float u_heightScaleScale;
uniform float u_triplanarSharpness;
uniform float u_triplanarScale;

layout(binding = 0) uniform sampler2D u_texDisplacement;

const float gridScale = 1.5f; // Prevent cutting grid at the screen edges

vec3 getDisplacement(vec3 worldPos, vec3 worldNormal) {
  vec3 absNormal = abs(worldNormal);
  vec3 weights = pow(absNormal, vec3(u_triplanarSharpness));
  weights /= (weights.x + weights.y + weights.z);

  vec3 scaledWorldPos = worldPos * u_triplanarScale;
  vec3 x = texture(u_texDisplacement, scaledWorldPos.zy).rgb;
  vec3 y = texture(u_texDisplacement, scaledWorldPos.xz).rgb;
  vec3 z = texture(u_texDisplacement, scaledWorldPos.xy).rgb;

  return x * weights.x + y * weights.y + z * weights.z;
}

void main() {
  vec2 uv = a_pos.xy * gridScale * 0.5f + 0.5f;
  vec3 viewVec = calcViewVec(uv, u_pvInvLocal);
  Ray ray = Ray(u_camPos, normalize(viewVec));
  float waterRadius = u_planetRadius + u_heightScale * u_heightScaleScale;

  RayHitInfo hitInfo = raySphere(u_planetCenter, waterRadius, ray);
  vec3 worldPos = ray.origin + ray.dir * hitInfo.distToSphere;
  vec3 sphereDir = normalize(worldPos - u_planetCenter);

  float viewDist = length(worldPos - u_camPos);
  // float fadeStart = 1000.f;
  // float fadeEnd = 7000.f;
  // float distFactor = 1.f - smoothstep(fadeStart, fadeEnd, viewDist);

  vec3 displacement = getDisplacement(worldPos, sphereDir);
  vec3 displacementFinal = displacement;
  worldPos += sphereDir * displacementFinal;

	gl_Position = u_proj * (u_localView * (u_localTranslation * vec4(worldPos, 1.f)));
  gl_Position.z = linearDepthToLogDepth(gl_Position.w, u_camFar);

  v_worldPos = worldPos;
  v_sphereDir = sphereDir;
  v_viewDist = viewDist;
  v_ray = ray;
}

