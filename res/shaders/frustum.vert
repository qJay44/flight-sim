#version 460 core

layout (location = 0) in vec3 a_pos;

out float v_logW;

uniform mat4 u_proj;
uniform mat4 u_localView;
uniform mat4 u_localTranslation;

void main() {
	gl_Position = u_proj * (u_localView * (u_localTranslation * vec4(a_pos, 1.f)));

  v_logW = gl_Position.w;
}

