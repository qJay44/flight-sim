#version 460 core

layout(location = 0) in vec3 inPos;

out vec3 v_worldPos;

uniform mat4 u_model;
uniform mat4 u_camPV;
uniform vec3 u_camPos;

void main() {
  vec4 worldPos = u_model * vec4(inPos, 1.f);
  vec4 clipSpace = u_camPV * worldPos;

  v_worldPos = worldPos.xyz;

  gl_Position = clipSpace.xyzw;
}

