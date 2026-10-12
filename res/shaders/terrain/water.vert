#version 460 core

#include "../common.glsl"

layout(location = 0) in vec3 a_pos;

out vec3 v_worldPos;
out vec3 v_sphereDir;
out vec3 v_viewVec;
out float v_viewDist;
out float v_depth;
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
uniform float u_tileScale;
uniform float u_displacementScale;

layout(binding = 0) uniform sampler2D u_texDisplacement;

const float gridScale = 1.5f; // Prevent cutting grid at the screen edges

vec3 calcViewVec(vec2 uv) {
  vec2 ndc = uv * 2.f - 1.f;
  vec4 clipPos = vec4(ndc, -1.f, 1.f);
  vec4 worldPos = u_pvInvLocal * clipPos;
  worldPos /= worldPos.w;

  return worldPos.xyz;
}

void main() {
  vec2 uv = a_pos.xy * gridScale * 0.5f + 0.5f;
  vec3 viewVec = calcViewVec(uv);
  Ray ray = Ray(u_camPos, normalize(viewVec));
  float waterRadius = u_planetRadius + u_heightScale * u_heightScaleScale;

  RayHitInfo hitInfo = raySphere(u_planetCenter, waterRadius, ray);
  vec3 worldPos = ray.origin + ray.dir * hitInfo.distToSphere;
  vec3 sphereDir = normalize(worldPos - u_planetCenter);

  float viewDist = length(worldPos - u_camPos);
  float fadeStart = 1000.f;
  float fadeEnd = 7000.f;
  float distFactor = 1.f - smoothstep(fadeStart, fadeEnd, viewDist);

  vec2 equatorUV = vec2(atan(sphereDir.z, sphereDir.x), asin(sphereDir.y)) * u_tileScale;
  vec2 polarUV = sphereDir.xz * u_tileScale;
  float poleBlend = smoothstep(0.6f, 0.8f, abs(sphereDir.y));

  vec3 waterBottom = worldPos;
  vec3 displacementEquator = texture(u_texDisplacement, equatorUV).rgb;
  vec3 displacementPolar = texture(u_texDisplacement, polarUV).rgb;
  vec3 displacement = mix(displacementEquator, displacementPolar, poleBlend);
  worldPos += sphereDir * displacement * u_displacementScale * distFactor;

	gl_Position = u_proj * (u_localView * (u_localTranslation * vec4(worldPos, 1.f)));

  float C = 0.001;
  float distNorm = log(C * gl_Position.w + 1.0) / log(C * u_camFar + 1.0);
  gl_Position.z = (distNorm * 2.0 - 1.0) * gl_Position.w;

  v_worldPos = worldPos;
  v_sphereDir = sphereDir;
  v_viewVec = viewVec;
  v_viewDist = viewDist;
  v_depth = length(worldPos - waterBottom);
  v_ray = ray;
}

