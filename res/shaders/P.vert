#version 460 core

layout (location = 0) in vec3 a_pos;

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
  v_uv = a_pos.zx * 0.5f + 0.5f;
  v_normal = vec3(0.f, 1.f, 0.f);
  v_color = vec3(0.f);

	gl_Position = u_proj * u_view * worldPos;
}

