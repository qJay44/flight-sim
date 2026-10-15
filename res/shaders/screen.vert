#version 460 core

layout(location = 0) in vec3 a_pos;

out vec2 v_uv;

void main() {
  v_uv = a_pos.xy * 0.5f + 0.5f;
	gl_Position = vec4(a_pos.xy, 1.f, 1.f);
}

