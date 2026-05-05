#version 460 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTex;

out vec2 v_uv;

uniform mat4 u_camProj;
uniform mat4 u_camView;
uniform mat4 u_model;
uniform float u_lightRadius;

void main() {
  v_uv = inTex;

  vec3 worldPos = vec3(u_model * vec4(0.f, 0.f, 0.f, 1.f));
  vec4 viewPos = u_camView * vec4(worldPos, 1.f);

  viewPos.xy += inPos.xy * u_lightRadius;

  vec4 clipSpace = u_camProj * viewPos;

  // gl_Position = clipSpace.xyww; // Ignore camera far plane
  gl_Position = clipSpace;
}

