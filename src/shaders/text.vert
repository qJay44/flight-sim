#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTex;

out vec2 v_uv;

uniform mat4 u_model;
uniform mat4 u_proj;

void main() {
  v_uv = inTex;
  gl_Position = u_proj * u_model * vec4(inPos, 1.f);
}

