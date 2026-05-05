#version 460 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTex;

out vec2 v_uv;

uniform mat4 u_model;

void main() {
  v_uv = inTex;

  vec3 worldPos = vec3(u_model * vec4(inPos, 1.f));

  gl_Position = vec4(worldPos.xy, 0.f, 1.f);
}

