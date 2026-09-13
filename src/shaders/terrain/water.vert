#version 460 core

#include "common.glsl"

layout(location = 0) in vec3 a_pos;

out vec3 v_sphereDir;
out vec3 v_worldPos;
out vec2 v_uv;
out flat int v_id;

uniform mat4 u_camProj;
uniform mat4 u_localView;
uniform mat4 u_localTranslation;
uniform float u_camFar;
uniform float u_planetRadius;
uniform float u_waveHeight;
uniform float u_waveScale;
uniform float u_radiusScale;

layout(binding = 1) uniform sampler2D u_texDisplacement;

void main() {
  vec2 uv = a_pos.xz * 0.5f + 0.5f;
  vec3 sphereDir = normalize(cubeToSphere(a_pos.xz, gl_InstanceID));
  vec3 pos = sphereDir * u_planetRadius * u_radiusScale;
  vec3 wave = texture(u_texDisplacement, uv).rgb;

  vec4 worldPos = vec4(pos + sphereDir * wave, 1.f);

  v_sphereDir = sphereDir;
  v_worldPos = worldPos.xyz;
  v_uv = uv;
  v_id = gl_InstanceID;

	gl_Position = u_camProj * (u_localView * (u_localTranslation * worldPos));

  float C = 0.001;
  float distNorm = log(C * gl_Position.w + 1.0) / log(C * u_camFar + 1.0);
  gl_Position.z = (distNorm * 2.0 - 1.0) * gl_Position.w;
}

