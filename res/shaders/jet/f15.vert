#version 460 core

#include "../common.glsl"

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex;
layout (location = 2) in vec3 a_normal;
layout (location = 3) in vec3 a_color;

out vec3 v_worldPos;
out vec3 v_color;
out vec2 v_uv;
out vec3 v_normal;

uniform mat4 u_proj;
uniform mat4 u_model;
uniform mat4 u_modelView;
uniform float u_camFar;

void main() {
  vec4 ndc = vec4(a_pos, 1.f);
  v_worldPos = (u_model * ndc).xyz;
  v_color = a_color;
  v_uv = a_tex;
  v_normal = mat3(u_model) * a_normal;

	gl_Position = u_proj * u_modelView * ndc;
  gl_Position.z = linearDepthToLogDepth(gl_Position.w, u_camFar);
}

