#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

out vec3 v_color;

uniform mat4 u_model;
uniform mat4 u_camPV;

void main() {
  v_color = inColor;
	gl_Position = u_camPV * u_model * vec4(inPos, 1.0f);
}

