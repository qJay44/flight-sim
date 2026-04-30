#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTex;
layout (location = 3) in vec3 inNormal;

out vec3 v_worldPos;
out vec3 v_color;
out vec2 v_uv;
out vec3 v_normal;

uniform mat3 u_localRotation;
uniform mat4 u_model;
uniform mat4 u_camPV;

void main() {
  vec4 worldPos = u_model * vec4(inPos, 1.f);
  v_worldPos = worldPos.xyz;
  v_color = inColor;
  v_uv = inTex;
  v_normal = mat3(u_model) * u_localRotation * inNormal;
	gl_Position = u_camPV * worldPos;
}

