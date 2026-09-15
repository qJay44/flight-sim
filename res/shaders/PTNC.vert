#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex;
layout (location = 2) in vec3 a_normal;
layout (location = 3) in vec3 a_color;

out vec3 v_worldPos;
out vec2 v_uv;
out vec3 v_normal;
out vec3 v_color;

uniform mat4 u_model;
uniform mat4 u_proj;
uniform mat4 u_view;

void main() {
  vec4 worldPos = u_model * vec4(a_pos, 1.f);

  v_worldPos = worldPos.xyz;
  v_color = a_color;
  v_uv = a_tex;
  v_normal = a_normal;

	gl_Position = u_proj * u_view * worldPos;
}

