#version 460 core

out vec4 FragColor;

in vec2 v_uv;

layout(binding = 0) uniform sampler2D u_texScreenColor;
layout(binding = 1) uniform sampler2D u_texScreenDepth;

void main() {
  vec3 color = texture(u_texScreenColor, v_uv).rgb;
  vec3 finalColor = color;

  finalColor = pow(finalColor, vec3(1.f / 2.2f));
  FragColor = vec4(finalColor, 1.f);
}

